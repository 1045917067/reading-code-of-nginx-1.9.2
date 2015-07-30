
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HTTP_UPSTREAM_H_INCLUDED_
#define _NGX_HTTP_UPSTREAM_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>
#include <ngx_event_pipe.h>
#include <ngx_http.h>


#define NGX_HTTP_UPSTREAM_FT_ERROR           0x00000002
#define NGX_HTTP_UPSTREAM_FT_TIMEOUT         0x00000004
#define NGX_HTTP_UPSTREAM_FT_INVALID_HEADER  0x00000008
#define NGX_HTTP_UPSTREAM_FT_HTTP_500        0x00000010
#define NGX_HTTP_UPSTREAM_FT_HTTP_502        0x00000020
#define NGX_HTTP_UPSTREAM_FT_HTTP_503        0x00000040
#define NGX_HTTP_UPSTREAM_FT_HTTP_504        0x00000080
#define NGX_HTTP_UPSTREAM_FT_HTTP_403        0x00000100
#define NGX_HTTP_UPSTREAM_FT_HTTP_404        0x00000200
#define NGX_HTTP_UPSTREAM_FT_UPDATING        0x00000400
#define NGX_HTTP_UPSTREAM_FT_BUSY_LOCK       0x00000800
#define NGX_HTTP_UPSTREAM_FT_MAX_WAITING     0x00001000
#define NGX_HTTP_UPSTREAM_FT_NOLIVE          0x40000000
#define NGX_HTTP_UPSTREAM_FT_OFF             0x80000000

#define NGX_HTTP_UPSTREAM_FT_STATUS          (NGX_HTTP_UPSTREAM_FT_HTTP_500  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_502  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_503  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_504  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_403  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_404)

#define NGX_HTTP_UPSTREAM_INVALID_HEADER     40


#define NGX_HTTP_UPSTREAM_IGN_XA_REDIRECT    0x00000002
#define NGX_HTTP_UPSTREAM_IGN_XA_EXPIRES     0x00000004
#define NGX_HTTP_UPSTREAM_IGN_EXPIRES        0x00000008
#define NGX_HTTP_UPSTREAM_IGN_CACHE_CONTROL  0x00000010
#define NGX_HTTP_UPSTREAM_IGN_SET_COOKIE     0x00000020
#define NGX_HTTP_UPSTREAM_IGN_XA_LIMIT_RATE  0x00000040
#define NGX_HTTP_UPSTREAM_IGN_XA_BUFFERING   0x00000080
#define NGX_HTTP_UPSTREAM_IGN_XA_CHARSET     0x00000100
#define NGX_HTTP_UPSTREAM_IGN_VARY           0x00000200


typedef struct {
    ngx_msec_t                       bl_time;
    ngx_uint_t                       bl_state;

    // HTTP/1.1 200 OK ��Ӧ�е�200
    ngx_uint_t                       status; //��mytest_process_status_line��ֵ��Դͷ��ngx_http_parse_status_line // HTTP/1.1 200 OK ��Ӧ�е�200
    ngx_msec_t                       response_time;
    ngx_msec_t                       connect_time;
    ngx_msec_t                       header_time;
    off_t                            response_length;

    ngx_str_t                       *peer;
} ngx_http_upstream_state_t;


typedef struct {
    ngx_hash_t                       headers_in_hash; //��ngx_http_upstream_init_main_conf�ж�ngx_http_upstream_headers_in��Ա����hash�õ�
    ngx_array_t                      upstreams; /* ngx_http_upstream_srv_conf_t */
} ngx_http_upstream_main_conf_t;

typedef struct ngx_http_upstream_srv_conf_s  ngx_http_upstream_srv_conf_t;

typedef ngx_int_t (*ngx_http_upstream_init_pt)(ngx_conf_t *cf,
    ngx_http_upstream_srv_conf_t *us);
typedef ngx_int_t (*ngx_http_upstream_init_peer_pt)(ngx_http_request_t *r,
    ngx_http_upstream_srv_conf_t *us);


typedef struct {
    ngx_http_upstream_init_pt        init_upstream;
    ngx_http_upstream_init_peer_pt   init;
    void                            *data;
} ngx_http_upstream_peer_t;

//server backend1.example.com weight=5;
/*
��weight = NUMBER - ���÷�����Ȩ�أ�Ĭ��Ϊ1��
��max_fails = NUMBER - ��һ��ʱ���ڣ����ʱ����fail_timeout���������ã��������������Ƿ����ʱ���������ʧ����������Ĭ��Ϊ1����������Ϊ0���Թرռ�飬��Щ������proxy_next_upstream��fastcgi_next_upstream��404���󲻻�ʹmax_fails���ӣ��ж��塣
��fail_timeout = TIME - �����ʱ���ڲ�����max_fails�����ô�С��ʧ�ܳ������������������������ܲ����ã�ͬ����ָ���˷����������õ�ʱ�䣨����һ�γ�������������֮ǰ����Ĭ��Ϊ10�룬fail_timeout��ǰ����Ӧʱ��û��ֱ�ӹ�ϵ����������ʹ��proxy_connect_timeout��proxy_read_timeout�����ơ�
��down - ��Ƿ�������������״̬��ͨ����ip_hashһ��ʹ�á�
��backup - (0.6.7�����)������еķǱ��ݷ�������崻���æ����ʹ�ñ����������޷���ip_hashָ�����ʹ�ã���
*/
typedef struct { //ngx_http_upstream_srv_conf_s->servers[]�еĳ�Ա   �����ռ�͸�ֵ��ngx_http_upstream_server
    ngx_str_t                        name; ////server 127.0.0.1:8080 max_fails=3  fail_timeout=30s;�е�uriΪ/
    ngx_addr_t                      *addrs; //server   127.0.0.1:8080 max_fails=3  fail_timeout=30s;�е�127.0.0.1
    ngx_uint_t                       naddrs;
    ngx_uint_t                       weight;
    ngx_uint_t                       max_fails;
    time_t                           fail_timeout;

    unsigned                         down:1;
    unsigned                         backup:1;
} ngx_http_upstream_server_t;


#define NGX_HTTP_UPSTREAM_CREATE        0x0001
#define NGX_HTTP_UPSTREAM_WEIGHT        0x0002
#define NGX_HTTP_UPSTREAM_MAX_FAILS     0x0004
#define NGX_HTTP_UPSTREAM_FAIL_TIMEOUT  0x0008
#define NGX_HTTP_UPSTREAM_DOWN          0x0010
#define NGX_HTTP_UPSTREAM_BACKUP        0x0020

/*
upstream backend {
    server backend1.example.com weight=5;
    server backend2.example.com:8080;
    server unix:/tmp/backend3;
}

server {
    location / {
        proxy_pass http://backend;
    }
}
*/ //�����ռ�Ͳ��ָ�ֵ��ngx_http_upstream_add  server backend1.example.com weight=5;����xxx_pass(proxy_pass ���� fastcgi_pass��)���ᴴ���ýṹ����ʾ���η�������ַ��Ϣ��
struct ngx_http_upstream_srv_conf_s { //upstream {}ģ��������Ϣ,�������൱��server{}һ������
//һ��upstream{}���ýṹ������,�����umcf(ngx_http_upstream_main_conf_t)->upstreams����������umcf��upstreamģ��Ķ��������ˡ�
    ngx_http_upstream_peer_t         peer;
    void                           **srv_conf; //��ֵ��ngx_http_upstream����ʾupstream{}�����Ķ��� srv{}����λ��
    //��¼��upstream{}�������serverָ� server backend1.example.com weight=5;
    ngx_array_t                     *servers;  /* ngx_http_upstream_server_t */ //ngx_http_upstream����ngx_http_upstream_add�д����ռ�

    ngx_uint_t                       flags; //NGX_HTTP_UPSTREAM_CREATE | NGX_HTTP_UPSTREAM_MAX_FAILS��
    ngx_str_t                        host; //upstream xxx {}�е�xxx
    u_char                          *file_name; //�����ļ�����
    ngx_uint_t                       line; //�����ļ��е��к�
    in_port_t                        port;
    in_port_t                        default_port;
    ngx_uint_t                       no_port;  /* unsigned no_port:1 */

#if (NGX_HTTP_UPSTREAM_ZONE)
    ngx_shm_zone_t                  *shm_zone;
#endif
};


typedef struct {
    ngx_addr_t                      *addr;
    ngx_http_complex_value_t        *value;
} ngx_http_upstream_local_t;

/*
��ʵ�ϣ�HTTP�������ģ����nginx.conf�ļ����ṩ�������������������ngx_http_upstream_conf_t�ṹ���еĳ�Ա�ġ�
*/ //�ڽ�����upstream{}���õ�ʱ�򣬴����ýṹ����location{}����
typedef struct { //upstream���ð���proxy fastcgi wcgi�ȶ��øýṹ
//����ngx_http_upstream_t�ṹ����û��ʵ��resolved��Աʱ��upstream����ṹ��Ż���Ч�����ᶨ�����η�����������
    ngx_http_upstream_srv_conf_t    *upstream; 

    ngx_msec_t                       connect_timeout;//����TCP���ӵĳ�ʱʱ�䣬ʵ���Ͼ���д�¼���ӵ���ʱ����ʱ���Եĳ�ʱʱ��
    ngx_msec_t                       send_timeout;//��������ĳ�ʱʱ�䡣ͨ������д�¼���ӵ���ʱ�������õĳ�ʱʱ��
    ngx_msec_t                       read_timeout;//������Ӧ�ĳ�ʱʱ�䡣ͨ�����Ƕ��¼���ӵ���ʱ�������õĳ�ʱʱ��
    ngx_msec_t                       timeout;
    ngx_msec_t                       next_upstream_timeout;

    size_t                           send_lowat; //TCP��SO_SNDLOWATѡ���ʾ���ͻ�����������
//�����˽���ͷ���Ļ�����������ڴ��С��ngx_http_upstream_t�е�buffer��������������ת����Ӧ�����λ�����buffering��־λΪ0
//�������ת����Ӧʱ����ͬ����ʾ���հ���Ļ�������С
    size_t                           buffer_size;
    size_t                           limit_rate;
//����buffering��־λΪ1������������ת����Ӧʱ��Ч���������õ�ngx_event_pipe_t�ṹ���busy_size��Ա��
    size_t                           busy_buffers_size;
/*
��buffering��־λΪ1ʱ����������ٶȿ��������ٶȣ����п��ܰ��������ε���Ӧ�洢����ʱ�ļ��У���max_temp_file_sizeָ������ʱ�ļ���
��󳤶ȡ�ʵ���ϣ���������ngx_event_pipe_t�ṹ���е�temp_file
*/
    size_t                           max_temp_file_size;
    size_t                           temp_file_write_size; //��ʾ���������е���Ӧд����ʱ�ļ�ʱһ��д���ַ�������󳤶�

    size_t                           busy_buffers_size_conf;
    size_t                           max_temp_file_size_conf;
    size_t                           temp_file_write_size_conf;

    ngx_bufs_t                       bufs;//�Ի�����Ӧ�ķ�ʽת�����η������İ���ʱ��ʹ�õ��ڴ��С
/*
���ngx_http_upstream_t�ṹ���б��������İ�ͷ��headers in��Ա��ignore_headers���԰��ն�����λʹ��upstream��ת����ͷʱ������ĳЩͷ��
�Ĵ�����Ϊ32λ���ͣ�������ignore_headers�����Ա�ʾ32����Ҫ�������账���ͷ����Ȼ��Ŀǰupstream���ƽ��ṩ8��λ���ں���8��HTTPͷ���Ĵ�
��������
#define NGX_HTTP_UPSTREAM_IGN_XA_REDIRECT    0x00000002
#define NGX_HTTP_UPSTREAM_IGN_XA_EXPIRES     0x00000004
#define NGX_HTTP_UPSTREAM_IGN_EXPIRES        0x00000008
#define NGX_HTTP_UPSTREAM_IGN_CACHE_CONTROL  0x00000010
#define NGX_HTTP_UPSTREAM_IGN_SET_COOKIE     0x00000020
#define NGX_HTTP_UPSTREAM_IGN_XA_LIMIT_RATE  0x00000040
#define NGX_HTTP_UPSTREAM_IGN_XA_BUFFERING   0x00000080
#define NGX_HTTP_UPSTREAM_IGN_XA_CHARSET     0x00000100
#define NGX_HTTP_UPSTREAM_IGN_VARY           0x00000200
*/
    ngx_uint_t                       ignore_headers;
/*
�Զ�����λ����ʾһЩ�����룬�������������Ӧʱ������Щ�����룬��ô��û�н���Ӧת�������οͻ���ʱ������ѡ����
һ�����η��������ط����󡣲μ�ngx_ht tp_upstream_next����
*/
    ngx_uint_t                       next_upstream;
/*
��buffering��־Ϊ1�������ת����Ӧʱ�����п��ܰ���Ӧ��ŵ���ʱ�ļ��С���ngx_http_upstream_t�е�store��־λΪ1ʱ��
store_access��ʾ��������Ŀ¼���ļ���Ȩ��
*/
    ngx_uint_t                       store_access;
    ngx_uint_t                       next_upstream_tries;
/*
����ת����Ӧ��ʽ�ı�־λ��bufferingΪ1ʱ��ʾ�򿪻��棬��ʱ��Ϊ���ε����ٿ������ε����٣��ᾡ�������ڴ���ߴ����л����������ε�
��Ӧ�����bufferingΪ0�����Ὺ��һ��̶���С���ڴ����Ϊ������ת����Ӧ
*/
    ngx_flag_t                       buffering; //��xxx_buffering��fastcgi_buffering  �Ƿ񻻳ɺ�˷�����Ӧ������İ���
    ngx_flag_t                       request_buffering;//�Ƿ񻻳ɿͻ�������İ��� XXX_request_buffering (����proxy_request_buffering fastcgi_request_buffering
    ngx_flag_t                       pass_request_headers;////�Ƿ�ת���ͻ������������������ͷ�������ȥ
    ngx_flag_t                       pass_request_body; ////�Ƿ�ת���ͻ�������������İ��嵽���ȥ

/*
��ʾ��־λ������Ϊ1ʱ����ʾ�����η���������ʱ�������Nginx�����οͻ��˼�������Ƿ�Ͽ���
Ҳ����˵����ʹ���οͻ��������ر������ӣ�Ҳ�����ж������η�������Ľ�������ngx_http_upstream_init_request
*/
    ngx_flag_t                       ignore_client_abort;
/*
������������Ӧ�İ�ͷʱ��������������õ�headers_in�ṹ���е�status_n���������400�������ͼ������error_page��ָ���Ĵ�������ƥ�䣬
���ƥ���ϣ�����error_page��ָ������Ӧ����������������η������Ĵ����롣���ngx_http_upstream_intercept_errors����
*/
    ngx_flag_t                       intercept_errors;
/*
buffering��־λΪ1�������ת����Ӧʱ�������塣��ʱ�����cyclic_temp_fileΪl�������ͼ������ʱ�ļ����Ѿ�ʹ�ù��Ŀռ䡣������
��cyclic_temp_f ile��Ϊ1
*/
    ngx_flag_t                       cyclic_temp_file;
    ngx_flag_t                       force_ranges;

    ngx_path_t                      *temp_path; //��buff ering��־λΪ1�������ת����Ӧʱ�������ʱ�ļ���·��
/*
��ת����ͷ����ʵ������ͨ��ngx_http_upstream_hide_headers_hash����������hide_headers��pass_headers��̬���鹹�������Ҫ���ص�HTTPͷ��ɢ�б�
*/
    ngx_hash_t                       hide_headers_hash;

/*
hide_headers��������ngx_array_t��̬���飨ʵ���ϣ�upstreamģ�齫��ͨ��hide_headers������hide_headers_hashɢ�б���
����upstreamģ��Ҫ��hide_headers������ΪNULL�����Ա���Ҫ��ʼ��hide_headers��Ա��upstreamģ���ṩ��
ngx_http_upstream_hide_headers hash��������ʼ��hide_headers�����������ںϲ���������ڡ�
*/
//��ת��������Ӧͷ����ngx_http_upstream_t��headers_in�ṹ���е�ͷ���������οͻ���ʱ�����ϣ��ĳЩͷ��ת�������Σ������õ�hide_headers��̬������
    ngx_array_t                     *hide_headers;
/*
��ת��������Ӧͷ����ngx_http_upstream_t��headers_in�ṹ���е�ͷ���������οͻ���ʱ��upstream����Ĭ�ϲ���ת���硰Date������Server��֮
���ͷ�������ȷʵϣ��ֱ��ת�����ǵ����Σ������õ�pass_headers��̬������
*/
    ngx_array_t                     *pass_headers;

    ngx_http_upstream_local_t       *local;//�������η�����ʱ���õı�����ַ

#if (NGX_HTTP_CACHE)
    ngx_shm_zone_t                  *cache_zone;
    ngx_http_complex_value_t        *cache_value;

    ngx_uint_t                       cache_min_uses;
    ngx_uint_t                       cache_use_stale;
    ngx_uint_t                       cache_methods;

    ngx_flag_t                       cache_lock;
    ngx_msec_t                       cache_lock_timeout;
    ngx_msec_t                       cache_lock_age;

    ngx_flag_t                       cache_revalidate;

    ngx_array_t                     *cache_valid;
    ngx_array_t                     *cache_bypass;
    ngx_array_t                     *no_cache;
#endif

/*
��ngx_http_upstream_t�е�store��־λΪ1ʱ�������Ҫ�����ε���Ӧ��ŵ��ļ��У�store_lengths����ʾ���·���ĳ��ȣ���store_values��ʾ���·��
*/
    ngx_array_t                     *store_lengths;
    ngx_array_t                     *store_values;

#if (NGX_HTTP_CACHE)
    signed                           cache:2;
#endif
//xxx_store(����scgi_store)  on | off |path   ֻҪ����off,store��Ϊ1����ֵ��ngx_http_fastcgi_store
//�ƶ��˴洢ǰ���ļ���·��������onָ���˽�ʹ��root��aliasָ����ͬ��·����off��ֹ�洢�����⣬�����п���ʹ�ñ���ʹ·��������ȷ��fastcgi_store /data/www$original_uri;
    signed                           store:2;//��ĿǰΪֹ��store��־λ��������ngx_http_upstream_t�е�store��ͬ����ֻ��o��1��ʹ�õ�
/*
�����intercept_errors��־λ������400���ϵĴ����뽫����error_page�ȽϺ����д���ʵ������������ǿ�����һ����������ģ������intercept_404
��־λ��Ϊ1�������η���404ʱ��ֱ��ת���������������Σ�������ȥ��error_page���бȽ�
*/
    unsigned                         intercept_404:1;
/*
���ñ�־λΪ1ʱ���������ngx_http_upstream_t��headers_in�ṹ�����"X-Accel-Buffering"ͷ��������ֵ����yes��no�����ı�buffering
��־λ������ֵΪyesʱ��buffering��־λΪ1����ˣ�change_bufferingΪ1ʱ���п��ܸ������η��������ص���Ӧͷ������̬�ؾ���������
���������Ȼ�����������������
*/
    unsigned                         change_buffering:1;

#if (NGX_HTTP_SSL)
    ngx_ssl_t                       *ssl;
    ngx_flag_t                       ssl_session_reuse;

    ngx_http_complex_value_t        *ssl_name;
    ngx_flag_t                       ssl_server_name;
    ngx_flag_t                       ssl_verify;
#endif

    ngx_str_t                        module; //ʹ��upstream��ģ�����ƣ������ڼ�¼��־
} ngx_http_upstream_conf_t;

//ngx_http_upstream_headers_in�еĸ�����Ա
typedef struct {
    ngx_str_t                        name;
    ngx_http_header_handler_pt       handler; //��mytest_upstream_process_header��ִ��
    ngx_uint_t                       offset;
    ngx_http_header_handler_pt       copy_handler; //ngx_http_upstream_process_headers��ִ��
    ngx_uint_t                       conf;
    ngx_uint_t                       redirect;  /* unsigned   redirect:1; */
} ngx_http_upstream_header_t;


//�ο�mytest_upstream_process_header->ngx_http_parse_header_line
//ngx_http_upstream_headers_in
typedef struct {
    ngx_list_t                       headers;

    //��mytest_process_status_line��ֵ��Դͷ��ngx_http_parse_status_line // HTTP/1.1 200 OK ��Ӧ�е�200
    ngx_uint_t                       status_n;// HTTP/1.1 200 OK ��Ӧ�е�200

    //��������ΪHTTP/1.1 200 OK �е�"200 OK "
    ngx_str_t                        status_line; //��mytest_process_status_line��ֵ��Դͷ��ngx_http_parse_status_line // HTTP/1.1 200 OK ��Ӧ�е�200

    ngx_table_elt_t                 *status;//��mytest_process_status_line��ֵ��Դͷ��ngx_http_parse_status_line
    ngx_table_elt_t                 *date;
    ngx_table_elt_t                 *server;
    ngx_table_elt_t                 *connection;

    ngx_table_elt_t                 *expires;
    ngx_table_elt_t                 *etag;
    ngx_table_elt_t                 *x_accel_expires;
    ngx_table_elt_t                 *x_accel_redirect;
    ngx_table_elt_t                 *x_accel_limit_rate;

    ngx_table_elt_t                 *content_type;
    ngx_table_elt_t                 *content_length;

    ngx_table_elt_t                 *last_modified;
    ngx_table_elt_t                 *location;
    ngx_table_elt_t                 *accept_ranges;
    ngx_table_elt_t                 *www_authenticate;
    ngx_table_elt_t                 *transfer_encoding;
    ngx_table_elt_t                 *vary;

#if (NGX_HTTP_GZIP)
    ngx_table_elt_t                 *content_encoding;
#endif

    ngx_array_t                      cache_control;
    ngx_array_t                      cookies;

    off_t                            content_length_n;
    time_t                           last_modified_time;

    unsigned                         connection_close:1;
    unsigned                         chunked:1;
} ngx_http_upstream_headers_in_t;

//resolved�ṹ�壬�����������η������ĵ�ַ
typedef struct { //�����ռ�͸�ֵ��ngx_http_fastcgi_eval
    ngx_str_t                        host; //sockaddr��Ӧ�ĵ�ַ�ַ���,��a.b.c.d
    in_port_t                        port; //�˿�
    ngx_uint_t                       no_port; /* unsigned no_port:1 */

    ngx_uint_t                       naddrs; //��ַ������
    ngx_addr_t                      *addrs; 

    struct sockaddr                 *sockaddr; //���η������ĵ�ַ
    socklen_t                        socklen; //sizeof(struct sockaddr_in);

    ngx_resolver_ctx_t              *ctx;
} ngx_http_upstream_resolved_t;


typedef void (*ngx_http_upstream_handler_pt)(ngx_http_request_t *r,
    ngx_http_upstream_t *u);

/*
upstream��3�ִ���������Ӧ����ķ�ʽ����HTTPģ����θ���
upstreamʹ����һ�ַ�ʽ�������ε���Ӧ�����أ��������ngx_http_request_t�ṹ����
subrequest_in_memory��־λΪ1ʱ�������õ�1�ַ�ʽ����upstream��ת����Ӧ����
�����Σ���HTTPģ��ʵ�ֵ�input_filter����������壻��subrequest_in_memoryΪ0ʱ��
upstream��ת����Ӧ���塣��ngx_http_upstream_conf t���ýṹ���е�buffering��־λΪ1
����������������ڴ�ʹ����ļ����ڻ������ε���Ӧ���壬����ζ�������ٸ��죻��buffering
Ϊ0ʱ����ʹ�ù̶���С�Ļ�����������������ܵ�buffer����������ת����Ӧ���塣
    ע��  ������8���ص������У�ֻ��create_request��process_header��finalize_request
�Ǳ���ʵ�ֵģ�����5���ص�����-input_filter init��input_filter��reinit_request��abort
request��rewrite redirect�ǿ�ѡ�ġ���12�»���ϸ�������ʹ����5����ѡ�Ļص���������
�⣬��8�������Ļص�������5.2�ڡ�
*/

/*
ngx_http_upstream_create��������ngx_http_upstream_t�ṹ�壬���еĳ�Ա����Ҫ����HTTPģ���������á�
����upstream����ʹ��ngx_http_upstream_init����
*/ //FastCGI memcached  uwsgi  scgi proxyģ���������ö����ڸýṹ��
//ngx_http_request_t->upstream �д�ȡ
struct ngx_http_upstream_s { //�ýṹ�еĲ��ֳ�Ա�Ǵ�upstream{}�е������������(ngx_http_upstream_conf_t)��ȡ��
    //������¼��Ļص�������ÿһ���׶ζ��в�ͬ��read event handler
    ngx_http_upstream_handler_pt     read_event_handler;
    //����д�¼��Ļص�������ÿһ���׶ζ��в�ͬ��write event handler
    ngx_http_upstream_handler_pt     write_event_handler;

    //��ʾ���������η�������������ӡ� 
    ngx_peer_connection_t            peer;

    /*
     �������οͻ���ת����Ӧʱ��ngx_http_request_t�ṹ���е�subrequest_in_memory��־סΪ0����������˻�������Ϊ�������ٸ��죨conf
     �����е�buffering��־λΪ1������ʱ��ʹ��pipe��Ա��ת����Ӧ����ʹ�����ַ�ʽת����Ӧʱ��������HTTPģ����ʹ��upstream����ǰ����
     pipe�ṹ�壬�����������ص�coredump����
     */
    ngx_event_pipe_t                *pipe;

    /* request_bufs��������ʲô������������η���������ʵ��create_request����ʱ��Ҫ������ 
    request_bufs������ķ�ʽ��ngx_buf_t��������������������ʾ������Ҫ���͵����η��������������ݡ�
    ���ԣ�HTTPģ��ʵ�ֵ�create_request�ص����������ڹ���reque st_bufg����
    */
    ngx_chain_t                     *request_bufs; //�������η�����������ͷ���ݷ����buf

    //�����������η�����Ӧ�ķ�ʽ
    ngx_output_chain_ctx_t           output; //������ݵĽṹ���������Ҫ���͵����ݣ��Լ����͵�output_filterָ��
    ngx_chain_writer_ctx_t           writer; //�ο�ngx_chain_writer������Ὣ���bufһ�������ӵ����
    //����ngx_output_chain��Ҫ���͵����ݶ���������Ȼ���ͣ�Ȼ������������ָ��ʣ�µĻ�û�е���writev���͵ġ�

    //upstream����ʱ�����������Բ�����
    /*
    conf��Ա������������upstreamģ�鴦������ʱ�Ĳ������������ӡ����͡����յĳ�ʱʱ��ȡ�
    ��ʵ�ϣ�HTTP�������ģ����nginx.conf�ļ����ṩ�������������������ngx_http_upstream_conf_t�ṹ���еĳ�Ա�ġ�
    �����г���3����ʱʱ��(connect_timeout  send_imeout read_timeout)�Ǳ���Ҫ���õģ���Ϊ����Ĭ��Ϊ0����������ý���Զ�޷������η�����������TCP���ӣ���Ϊconnect timeoutֵΪ0����
    */
    ngx_http_upstream_conf_t        *conf; //ʹ��upstream����ʱ�ĸ�������  ����fastcgi��ֵ��ngx_http_fastcgi_handler
#if (NGX_HTTP_CACHE)
    ngx_array_t                     *caches;
#endif

    /*
     HTTPģ����ʵ��process_header����ʱ�����ϣ��upstreamֱ��ת����Ӧ������Ҫ�ѽ���������Ӧͷ������ΪHTTP����Ӧͷ����ͬʱ��Ҫ
     �Ѱ�ͷ�е���Ϣ���õ�headers_in�ṹ���У����������headers_in�����õ�ͷ����ӵ�Ҫ���͵����οͻ��˵���Ӧͷ��headers_out��
     */
    ngx_http_upstream_headers_in_t   headers_in;  //��Ŵ����η��ص�ͷ����Ϣ��

    //ͨ��resolved����ֱ��ָ�����η�������ַ�����ڽ�����������  �����͸�ֵ��ngx_http_xxx_eval(����ngx_http_fastcgi_eval ngx_http_proxy_eval)
    ngx_http_upstream_resolved_t    *resolved; //����������fastcgi_pass   127.0.0.1:9000;������ַ������ݣ������б����

    ngx_buf_t                        from_client;

    /*
    buffer��Ա�洢���������η�������������Ӧ���ݣ��������ᱻ���ã����Ծ������ж������壺
    a����ʹ��process_header��������������Ӧ�İ�ͷʱ��buffer�н��ᱣ����������Ӧ��ͷ��
    b���������buffering��ԱΪ1�����Ҵ�ʱupstream��������ת�����εİ���ʱ��bufferû�����壻
    c����buffering��־סΪ0ʱ��buffer�������ᱻ���ڷ����ؽ������εİ��壬����������ת����
    d����upstream��������ת�����ΰ���ʱ��buffer�ᱻ���ڷ����������εİ��壬HTTPģ��ʵ�ֵ�input_filter������Ҫ��ע��

    �������η�������Ӧ��ͷ�Ļ��������ڲ���Ҫ����Ӧֱ��ת�����ͻ��ˣ�����buffering��־λΪ0�������ת������ʱ�����հ���Ļ���
����Ȼʹ��buffer��ע�⣬���û���Զ���input_filter����������壬����ʹ��buffer�洢ȫ���İ��壬��ʱbuf fer�����㹻�����Ĵ�С
��ngx_http_upstream_conf_t�ṹ���е�buffer_size��Ա����
    */
//��ȡ���η��ص����ݵĻ�������Ҳ����proxy��FCGI���ص����ݡ���������httpͷ����Ҳ������body���֡���body���ֻ��event_pipe_t��preread_bufs�ṹ��Ӧ����������Ԥ����buf����ʵ��i��С�Ķ����ġ�
    ngx_buf_t                        buffer; //�����η��������յ������ڸ�buffer���������ε�����������request_bufs��
    //��ʾ�������η���������Ӧ����ĳ���
    off_t                            length; //Ҫ���͸��ͻ��˵����ݴ�С������Ҫ��ȡ��ô������� 

    /*
out_bufs�����ֳ������в�ͬ�����壺
�ٵ�����Ҫת�����壬��ʹ��Ĭ�ϵ�input_filter������Ҳ����ngx_http_upstream_non_buffered_filter�������������ʱ��out bufs����ָ����Ӧ���壬
��ʵ�ϣ�out bufs�����л�������ngx_buf_t��������ÿ����������ָ��buffer�����е�һ���֣��������һ���־���ÿ�ε���recv�������յ���һ��TCP����
�ڵ���Ҫת����Ӧ���嵽����ʱ��buffering��־λΪO�����������������ȣ����������ָ����һ��������ת����Ӧ���������ʱ���ڽ��������εĻ�����Ӧ
     */
    ngx_chain_t                     *out_bufs;
    /*
    ����Ҫת����Ӧ���嵽����ʱ��buffering��־λΪo�����������������ȣ�������ʾ��һ��������ת����Ӧʱû�з����������
     */
    ngx_chain_t                     *busy_bufs;//������ngx_http_output_filter������out_bufs�����������ƶ��������������Ϻ󣬻��ƶ���free_bufs
    /*
    ����������ڻ���out_bufs���Ѿ����͸����ε�ngx_buf_t�ṹ�壬��ͬ��Ӧ����buffering��־λΪ0���������������ȵĳ���
     */
    ngx_chain_t                     *free_bufs;//���еĻ����������Է���

/*
input_filter init��input_filter�ص�����
    input_filter_init��input_filter���������������ڴ������ε���Ӧ���壬��Ϊ�������
ǰHTTPģ�������Ҫ��һЩ��ʼ�����������磬����һЩ�ڴ����ڴ�Ž������м�״̬
�ȣ���ʱupstream���ṩ��input_filter_init��������input_filter��������ʵ�ʴ�������
�������������ص�����������ѡ����ʵ�֣�������Ϊ��������������ʵ��ʱ��upstream
ģ����Զ���������ΪԤ�÷��������Ľ���������upstream��3�ִ������ķ�ʽ������
upstreamģ��׼����3��input_filter_init��input_filter����������ˣ�һ����ͼ�ض���mput_
filter init��input_filter����������ζ�����Ƕ�upstreamģ���Ĭ��ʵ���ǲ�����ģ����Բ�
Ҫ�ض���ù��ܡ�
    �ڶ�������£��������³�����������ʵ��input_filter������
    (1)��ת��������Ӧ�����ε�ͬʱ����Ҫ��һЩ���⴦��
    ���磬ngx_http_memcached_ moduleģ��Ὣʵ����memcachedʵ�ֵ����η���������
����Ӧ���壬ת�������ε�HTTP�ͻ����ϡ������������У���ģ��ͨ���ض����˵�input_
filter���������memcachedЭ���°���Ľ�������������ȫ�������͸��TCP����
    (2)���������ϡ����μ�ת����Ӧʱ��������ȴ�������ȫ����������Ӧ��ſ�ʼ����
����
    �ڲ�ת����Ӧʱ��ͨ���Ὣ��Ӧ���������ڴ��н����������ͼ���յ���������Ӧ��
����������������Ӧ���ܻ�ǳ������ռ�ô����ڴ档���ض�����input_filter�����󣬿�
��ÿ������һ���ְ��壬���ͷ�һЩ�ڴ档
    �ض���input_filter�����������һЩ����������ȡ���ս��յ��İ����Լ�������Ż�
����ʹ�ù̶���С���ڴ滺���������ظ�ʹ�õȡ�ע�⣬���µ����Ӳ����漰input_filter��
�������߿����ڵ�12�����ҵ�input_filter������ʹ�÷�ʽ��
*/
//�������ǰ�ĳ�ʼ������������data�������ڴ����û����ݽṹ����ʵ���Ͼ��������input_filter_ctxָ��
    ngx_int_t                      (*input_filter_init)(void *data); //ngx_http_XXX_input_filter_init(��ngx_http_fastcgi_input_filter_init)
/* �������ķ���������data�������ڴ����û����ݽṹ����ʵ���Ͼ��������input_filter_ctxָ�룬��bytes��ʾ���ν��յ��İ��峤�ȡ�
����NGX ERRORʱ��ʾ����������������Ҫ���������򶼽�����upstream����

������ȡ��˵����ݣ���bufferingģʽ��ngx_http_upstream_non_buffered_filter��ngx_http_memcached_filter�ȡ���������ĵ���ʱ��: 
ngx_http_upstream_process_non_buffered_upstream�ȵ���ngx_unix_recv���յ�upstream���ص����ݺ�͵����������Э��ת��������Ŀǰת�����ࡣ
*/
    ngx_int_t                      (*input_filter)(void *data, ssize_t bytes); //ngx_http_xxx_non_buffered_filter(��ngx_http_fastcgi_non_buffered_filter)
//���ڴ���HTTPģ���Զ�������ݽṹ����input_filter_init��input_filter�������ص�ʱ����Ϊ�������ݹ�ȥ
    void                            *input_filter_ctx;//ָ�������������������

#if (NGX_HTTP_CACHE)
    ngx_int_t                      (*create_key)(ngx_http_request_t *r);
#endif
    //���췢�����η���������������
    /*
    create_request�ص�����
    create_request�Ļص�������򵥣�����ֻ���ܱ�����1�Σ����������upstream��
ʧ�����Ի��ƵĻ��������12�£�����ͼ5-3��ʾ������򵥵ؽ���һ��ͼ5-3�е�ÿһ
�����裺
    1)��Nginx��ѭ�����������ѭ����ָ8.5���ᵽ��ngx_worker_process_cycle�������У��ᶨ�ڵص����¼�ģ�飬�Լ���Ƿ��������¼�������
    2)�¼�ģ���ڽ��յ�HTTP���������HTIP���������������ա�������HTTPͷ������Ӧ����mytestģ�鴦����ʱ�����mytestģ
    ���ngx_http_mytest_handler������
    3)����mytestģ���ʱ�����5.1.2�ڡ�5.1.4�������г��Ĳ��衣
    4)����ngx_http_up stream_init��������upstream��
    5) upstreamģ���ȥ����ļ����棬����������Ѿ��к��ʵ���Ӧ�������ֱ�ӷ��ػ��棨��Ȼ��������ʹ�÷�������ļ������ǰ���£���
    Ϊ���ö��߷�������upstream���ƣ����½������ἰ�ļ����档
    6)�ص�mytestģ���Ѿ�ʵ�ֵ�create_request�ص�������
    7) mytestģ��ͨ������r->upstream->request_bufs�Ѿ������÷���ʲô�����������η�������
    8) upstreamģ�齫����5.1.3���н��ܹ���resolved��Ա�������resolved��Ա�Ļ����͸��������ú����η������ĵ�ַr->upstream->peer��Ա��
    9)����������TCP�׽��ֽ������ӡ�
    10)���������Ƿ����ɹ������������ӵ�connect�����������̷��ء�
    II) ngx_http_upstreamL init���ء�
    12) mytestģ���ngx_http_mytest_handler��������NGX DONE��
    13)���¼�ģ�鴦�������������¼��󣬽�����Ȩ������Nginx��ѭ����
    */ //���ﶨ���mytest_upstream_create_request�������ڴ������͸����η�������HTTP����upstreamģ�齫��ص��� 
    //��ngx_http_upstream_init_request��ִ��  HTTPģ��ʵ�ֵ�ִ��create_request�������ڹ��췢�����η�����������
    //ngx_http_xxx_create_request(����ngx_http_fastcgi_create_request)
    ngx_int_t                      (*create_request)(ngx_http_request_t *r);//���ɷ��͵����η����������󻺳壨����һ����������

/*
reinit_request���ܻᱻ��λص����������õ�ԭ��ֻ��һ���������ڵ�һ����ͼ�����η�������������ʱ������������ڸ����쳣ԭ��ʧ�ܣ�
��ô�����upstream��conf�����Ĳ���Ҫ���ٴ��������η�����������ʱ�ͻ����reinit_request�����ˡ�ͼ5-4�����˵��͵�reinit_request���ó�����
����򵥵ؽ���һ��ͼ5-4���г��Ĳ��衣
    1) Nginx��ѭ���лᶨ�ڵص����¼�ģ�飬����Ƿ��������¼�������
    2)�¼�ģ����ȷ�������η�������TCP���ӽ����ɹ��󣬻�ص�upstreamģ�����ط�������
    3) upstream�����ʱ���r->upstream->request_sent��־λ��Ϊl����ʾ�����Ѿ������ɹ��ˣ����ڿ�ʼ�����η����������������ݡ�
    4)�����������η�������
    5)���ͷ�����Ȼ���������ģ�ʹ�������������׽��֣��������̷��ء�
    6) upstreamģ�鴦���2���е�TCP���ӽ����ɹ��¼���
    7)�¼�ģ�鴦���걾�������¼��󣬽�����Ȩ������Nginx��ѭ����
    8) Nginx��ѭ���ظ���1���������¼�ģ���������¼���
    9)��ʱ��������������η�����������TCP�����Ѿ��쳣�Ͽ�����ô�¼�ģ���֪ͨupstreamģ�鴦������
    10)�ڷ������Դ�����ǰ���£�upstreamģ��������ԥ���ٴ������������׽�����ͼ�������ӡ�
    11)���������Ƿ����ɹ������̷��ء�
    12)��ʱ���r->upstream->request_sent��־λ���ᷢ�����Ѿ�����Ϊ1�ˡ�
    13)���mytestģ��û��ʵ��reinit_request��������ô�ǲ���������ġ������reinit_request��ΪNULL��ָ�룬�ͻ�ص�����
    14) mytestģ����reinit_request�д������Լ������顣
    15)�����ܵ�9���е�TCP���ӶϿ��¼���������Ȩ�������¼�ģ�顣
    16)�¼�ģ�鴦���걾�������¼��󣬽�������Ȩ��Nginx��ѭ����
*/ //�����η�������ͨ��ʧ�ܺ�����������Թ�����Ҫ�ٴ������η������������ӣ�������reinit_request����
    //�����upstream�ص�ָ���Ǹ���ģ�����õģ�����ngx_http_fastcgi_handler����������fcgi����ػص�������
    //ngx_http_XXX_reinit_request(ngx_http_fastcgi_reinit_request)
    ngx_int_t                      (*reinit_request)(ngx_http_request_t *r);//�ں�˷����������õ�����£���create_request���ڶ��ε���֮ǰ��������

/*
�յ����η���������Ӧ��ͻ�ص�process_header���������process_header����NGXAGAIN����ô���ڸ���upstream��û���յ���������Ӧ��ͷ��
��ʱ�����ӱ���upstream������˵���ٴν��յ����η�����������TCP��ʱ���������process_header��������ֱ��process_header��������
��NGXAGAINֵ��һ�׶βŻ�ֹͣ

process_header�ص�����process_header�����ڽ������η��������صĻ���TCP����Ӧͷ���ģ���ˣ�process_header���ܻᱻ��ε��ã�
���ĵ��ô�����process_header�ķ���ֵ�йء���ͼ5-5��ʾ�����process_header����NGX_AGAIN������ζ�Ż�û�н��յ���������Ӧͷ����
����ٴν��յ����η�����������TCP���������������ͷ������Ȼ����process_header��������ͼ5-6�У����process_header����NGX_OK
������������NGX_AGAIN��ֵ������ô��������ӵĺ��������н������ٴε���process_header��
 process header�ص�����������ͼ
����򵥵ؽ���һ��ͼ5-5���г��Ĳ��衣
    1) Nginx��ѭ���лᶨ�ڵص����¼�ģ�飬����Ƿ��������¼�������
    2)�¼�ģ����յ����η�������������Ӧʱ����ص�upstreamģ�鴦��
    3) upstreamģ����ʱ���Դ��׽��ֻ������ж�ȡ���������ε�TCP����
    4)��ȡ����Ӧ���ŵ�r->upstream->bufferָ����ڴ��С�ע�⣺��δ��������Ӧͷ��ǰ������ν��յ��ַ��������н��������ε�
    ��Ӧ���������ش�ŵ�r->upstream->buffer�������С���ˣ��ڽ���������Ӧ��ͷʱ�����buffer������ȫ��ȴ��û�н�������������Ӧ
    ͷ����Ҳ����˵��process_header -ֱ�ڷ���NGX_AGAIN������ô����ͻ����
    5)����mytestģ��ʵ�ֵ�process_header������
    6) process_header����ʵ���Ͼ����ڽ���r->upstream->buffer����������ͼ����ȡ����������Ӧͷ������Ȼ��������η�������Nginxͨ��HTTPͨ�ţ�
    ���ǽ��յ�������HTTPͷ������
    7)���process_header����NGX AGAIN����ô��ʾ��û�н�������������Ӧͷ�����´λ������process_header������յ���������Ӧ��
    8)����Ԫ�����Ķ�ȡ�׽��ֽӿڡ�
    9)��ʱ�п��ܷ����׽��ֻ������Ѿ�Ϊ�ա�
    10)����2���еĶ�ȡ������Ӧ�¼�������Ϻ󣬿���Ȩ�������¼�ģ�顣
    11)�¼�ģ�鴦���걾�������¼��󣬽�������Ȩ��Nginx��ѭ����
*/ //ngx_http_upstream_process_header��ngx_http_upstream_cache_send�����е���
/*
�������η�����������Ӧ�İ�ͷ������NGX_AGAIN��ʾ��ͷ��û�н�������������NGX_HTTP_UPSTREAM_INVALID_HEADER��ʾ��ͷ���Ϸ�������
NGX ERROR��ʾ���ִ��󣬷���NGX_OK��ʾ�����������İ�ͷ
*/ //ngx_http_fastcgi_process_header(ngx_http_XXX_process_header)
    ngx_int_t                      (*process_header)(ngx_http_request_t *r); //�������η������ظ��ĵ�һ��bit��ʱ���Ǳ���һ��ָ�����λظ����ص�ָ��
    void                           (*abort_request)(ngx_http_request_t *r);//�ڿͻ��˷��������ʱ�򱻵��� ngx_http_XXX_abort_request
   
/*
������ngx_http_upstream_init����upstream���ƺ��ڸ���ԭ�����۳ɹ�����ʧ�ܣ����¸���������ǰ�������finalize_request��
�����μ�ͼ5-1������finalize_request�����п��Բ����κ����飬������ʵ��finalize_request����������Nginx����ֿ�ָ����õ����ش���

���������ʱ������ص�finalize_request�������������ϣ����ʱ�ͷ���Դ�����
�ľ���ȣ�����ô���԰������Ĵ�����ӵ�finalize_request�����С������ж�����mytest_
upstream_finalize_request��������������û���κ���Ҫ�ͷŵ���Դ�����Ը÷���û�������
��ʵ�ʹ�����ֻ����Ϊupstreamģ��Ҫ�����ʵ��finalize_request�ص�����
*/ //����upstream����ʱ����  ngx_http_XXX_finalize_request
    void                           (*finalize_request)(ngx_http_request_t *r,
                                         ngx_int_t rc); //�������ʱ����� //��Nginx��ɴ����η���������ظ��Ժ󱻵���
/*
���ض���URL�׶Σ����ʵ����rewrite_redirect�ص���������ô��ʱ�����rewrite_redirect��ע�⣬���²��漰rewrite_redirect������
����Ȥ�Ķ��߿��Բ鿴upstreamģ���ngx_http_upstream_rewrite_ location���������upstreamģ����յ�������������Ӧͷ����
������HTTPģ���process_header�ص��������������Ķ�Ӧ��Location��ͷ�����õ���ngx_http_upstream_t�е�headers in��Աʱ��
ngx_http_up stre am_proces s_headers�����������յ���rewrite��redirect��������12.5.3��ͼ12-5�ĵ�8������
��ˣ�rewrite_ redirect��ʹ�ó����Ƚ��٣�����ҪӦ����HTTP�������ģ��(ngx_http_proxy_module)��
*/
//�����η��ص���Ӧ����Location����Refreshͷ����ʾ�ض���ʱ����ͨ��ngx_http_upstream_process_headers�������õ�����HTTPģ��ʵ�ֵ�rewrite redirect����
    ngx_int_t                      (*rewrite_redirect)(ngx_http_request_t *r,
                                         ngx_table_elt_t *h, size_t prefix);
    ngx_int_t                      (*rewrite_cookie)(ngx_http_request_t *r,
                                         ngx_table_elt_t *h);

    ngx_msec_t                       timeout;
    //���ڱ�ʾ������Ӧ�Ĵ����롢���峤�ȵ���Ϣ
    ngx_http_upstream_state_t       *state; //��r->upstream_states�����ȡ����ngx_http_upstream_init_request

    ngx_str_t                        method; //��ʹ���ļ�����ʱû������ //GET,HEAD,POST
    //schema��uri��Ա���ڼ�¼��־ʱ���õ�����������û������
    ngx_str_t                        schema; //����ǰ���http,https,mecached://  fastcgt://(ngx_http_fastcgi_handler)�ȡ�
    ngx_str_t                        uri;

#if (NGX_HTTP_SSL)
    ngx_str_t                        ssl_name;
#endif
    //Ŀǰ�������ڱ�ʾ�Ƿ���Ҫ������Դ���൱��һ����־λ��ʵ�ʲ�����õ�����ָ��ķ���
    ngx_http_cleanup_pt             *cleanup;
    //�Ƿ�ָ���ļ�����·���ı�־λ 
    //xxx_store(����scgi_store)  on | off |path   ֻҪ����off,store��Ϊ1����ֵ��ngx_http_fastcgi_store
//�ƶ��˴洢ǰ���ļ���·��������onָ���˽�ʹ��root��aliasָ����ͬ��·����off��ֹ�洢�����⣬�����п���ʹ�ñ���ʹ·��������ȷ��fastcgi_store /data/www$original_uri;
    unsigned                         store:1; //ngx_http_upstream_init_request��ֵ
    unsigned                         cacheable:1; //�Ƿ������ļ����棬���½�����cacheable��־סΪo�ĳ���
    unsigned                         accel:1;
    unsigned                         ssl:1; //�Ƿ����SSLЭ��������η�����
#if (NGX_HTTP_CACHE)
    unsigned                         cache_status:3;
#endif

    /*
    upstream��3�ִ���������Ӧ����ķ�ʽ����HTTPģ����θ���
    upstreamʹ����һ�ַ�ʽ�������ε���Ӧ�����أ��������ngx_http_request_t�ṹ����
    subrequest_in_memory��־λΪ1ʱ�������õ�1�ַ�ʽ����upstream��ת����Ӧ����
    �����Σ���HTTPģ��ʵ�ֵ�input_filter����������壻��subrequest_in_memoryΪ0ʱ��
    upstream��ת����Ӧ���塣��ngx_http_upstream_conf t���ýṹ���е�buffering��־λΪ1
    ����������������ڴ�ʹ����ļ����ڻ������ε���Ӧ���壬����ζ�������ٸ��죻��buffering
    Ϊ0ʱ����ʹ�ù̶���С�Ļ�����������������ܵ�buffer����������ת����Ӧ���塣
    
    ����ͻ���ת�����η������İ���ʱ�����á���bufferingΪ1ʱ����ʾʹ�ö���������Լ�������
    ����ת�����ε���Ӧ���塣��Nginx�����μ������Զ����Nginx�����οͻ��˼������ʱ����Nginx���ٸ����
    �ڴ�����ʹ�ô����ļ����������ε���Ӧ���壬����������ģ������Լ������η������Ĳ���ѹ������buffering
    Ϊ0ʱ����ʾֻʹ���������һ��buffer��������������ת����Ӧ����
    */
    unsigned                         buffering:1; //������ת�����ε���Ӧ����ʱ���Ƿ���������ڴ漰��ʱ�����ļ����ڻ������������͵����ε���Ӧ
    unsigned                         keepalive:1;
    unsigned                         upgrade:1;

/*
request_sent��ʾ�Ƿ��Ѿ������η��������������󣬵�request_sentΪ1ʱ����ʾupstream�����Ѿ������η�����������ȫ�����߲��ֵ�����
��ʵ�ϣ������־λ�������Ϊ��ʹ��ngx_output_chain��������������Ϊ�÷�����������ʱ���Զ���δ�������request_bufs�����¼������
Ϊ�˷�ֹ���������ظ����󣬱�����request_sent��־λ��¼�Ƿ���ù�ngx_output_chain����
*/
    unsigned                         request_sent:1;
/*
�����η���������Ӧ����Ϊ��ͷ�Ͱ�β���������Ӧֱ��ת�����ͻ��ˣ�header_sent��־λ��ʾ��ͷ�Ƿ��ͣ�header_sentΪ1ʱ��ʾ�Ѿ���
��ͷת�����ͻ����ˡ������ת����Ӧ���ͻ��ˣ���header_sentû������
*/
    unsigned                         header_sent:1;
};


typedef struct {
    ngx_uint_t                      status;
    ngx_uint_t                      mask;
} ngx_http_upstream_next_t;


typedef struct { //����ռ�͸�ֵ��ngx_http_upstream_param_set_slot�� �洢��ngx_http_fastcgi_loc_conf_t->params_source��
    ngx_str_t   key; //fastcgi_param  SCRIPT_FILENAME  aaa�е�SCRIPT_FILENAME
    ngx_str_t   value; //fastcgi_param  SCRIPT_FILENAME  aaa�е�aaa

    //ngx_http_fastcgi_init_params
    ngx_uint_t  skip_empty; //fastcgi_param  SCRIPT_FILENAME  aaa  if_not_empty������1   ��fastcgiΪ���ñ����������õط���ngx_http_fastcgi_create_request
} ngx_http_upstream_param_t;


ngx_int_t ngx_http_upstream_cookie_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
ngx_int_t ngx_http_upstream_header_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);

ngx_int_t ngx_http_upstream_create(ngx_http_request_t *r);
void ngx_http_upstream_init(ngx_http_request_t *r);
ngx_http_upstream_srv_conf_t *ngx_http_upstream_add(ngx_conf_t *cf,
    ngx_url_t *u, ngx_uint_t flags);
char *ngx_http_upstream_bind_set_slot(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
char *ngx_http_upstream_param_set_slot(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
ngx_int_t ngx_http_upstream_hide_headers_hash(ngx_conf_t *cf,
    ngx_http_upstream_conf_t *conf, ngx_http_upstream_conf_t *prev,
    ngx_str_t *default_hide_headers, ngx_hash_init_t *hash);


#define ngx_http_conf_upstream_srv_conf(uscf, module)                         \
    uscf->srv_conf[module.ctx_index]


extern ngx_module_t        ngx_http_upstream_module;
extern ngx_conf_bitmask_t  ngx_http_upstream_cache_method_mask[];
extern ngx_conf_bitmask_t  ngx_http_upstream_ignore_headers_masks[];


#endif /* _NGX_HTTP_UPSTREAM_H_INCLUDED_ */
