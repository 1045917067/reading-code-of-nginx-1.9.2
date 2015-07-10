
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HTTP_CORE_H_INCLUDED_
#define _NGX_HTTP_CORE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#if (NGX_THREADS)
#include <ngx_thread_pool.h>
#endif


#define NGX_HTTP_GZIP_PROXIED_OFF       0x0002
#define NGX_HTTP_GZIP_PROXIED_EXPIRED   0x0004
#define NGX_HTTP_GZIP_PROXIED_NO_CACHE  0x0008
#define NGX_HTTP_GZIP_PROXIED_NO_STORE  0x0010
#define NGX_HTTP_GZIP_PROXIED_PRIVATE   0x0020
#define NGX_HTTP_GZIP_PROXIED_NO_LM     0x0040
#define NGX_HTTP_GZIP_PROXIED_NO_ETAG   0x0080
#define NGX_HTTP_GZIP_PROXIED_AUTH      0x0100
#define NGX_HTTP_GZIP_PROXIED_ANY       0x0200


#define NGX_HTTP_AIO_OFF                0
#define NGX_HTTP_AIO_ON                 1
#define NGX_HTTP_AIO_THREADS            2

/*
�����NGX HTTP ACCESS PHASE�׶δ�������satisfy���������������
�����������������ש���������������������������������������������������������������������������������������
��satisfy�Ĳ��� ��    ����                                                                              ��
�ǩ��������������贈��������������������������������������������������������������������������������������
��              ��    NGX HTTP ACCESS PHASE�׶ο����кܶ�HTTPģ�鶼�Կ�������ķ���Ȩ�޸���Ȥ��         ��
��              ����ô����һ��Ϊ׼�أ���satisfy�Ĳ���Ϊallʱ����ЩHTTPģ�����ͬʱ�������ã����Ըý�    ��
��all           ��                                                                                      ��
��              ������ȫ����handler������ͬ��������ķ���Ȩ�ޣ����仰˵����һ�׶ε�����handler������    ��
��              ����ȫ������NGX OK������Ϊ������з���Ȩ��                                              ��
�ǩ��������������贈��������������������������������������������������������������������������������������
��              ��  ��all�෴������Ϊanyʱ��ζ����NGX��HTTP__ ACCESS��PHASE�׶�ֻҪ������һ��             ��
��              ��HTTPģ����Ϊ����Ϸ����Ͳ����ٵ�������HTTPģ���������ˣ�������Ϊ�����Ǿ��з���      ��
��              ��Ȩ�޵ġ�ʵ���ϣ���ʱ�������Щ���ӣ���������κ�һ��handler��������NGX��OK������Ϊ    ��
��              ��������з���Ȩ�ޣ����ĳһ��handler��������403����401������Ϊ����û�з���Ȩ�ޣ���     ��
��any           ��                                                                                      ��
��              ����Ҫ���NGX��HTTP��ACCESS��PHASE�׶ε�����handler������Ҳ����˵��any����������           ��
��              ����һ��handler����һ����Ϊ������з���Ȩ�ޣ�����Ϊ��һ�׶�ִ�гɹ�����������ִ�У���   ��
��              ��������һ��handler������Ϊû�з���Ȩ�ޣ���δ���Դ�Ϊ׼������Ҫ���������hanlder������  ��
��              ��all��any�е���&&���͡������Ĺ�ϵ                                                         ��
�����������������ߩ���������������������������������������������������������������������������������������
*/
#define NGX_HTTP_SATISFY_ALL            0
#define NGX_HTTP_SATISFY_ANY            1


#define NGX_HTTP_LINGERING_OFF          0
#define NGX_HTTP_LINGERING_ON           1
#define NGX_HTTP_LINGERING_ALWAYS       2


#define NGX_HTTP_IMS_OFF                0
#define NGX_HTTP_IMS_EXACT              1
#define NGX_HTTP_IMS_BEFORE             2


#define NGX_HTTP_KEEPALIVE_DISABLE_NONE    0x0002
#define NGX_HTTP_KEEPALIVE_DISABLE_MSIE6   0x0004
#define NGX_HTTP_KEEPALIVE_DISABLE_SAFARI  0x0008


typedef struct ngx_http_location_tree_node_s  ngx_http_location_tree_node_t;
typedef struct ngx_http_core_loc_conf_s  ngx_http_core_loc_conf_t;

//ͨ��ngx_http_core_listen�еĲ�������   ͨ��server{}"listen"����������������ĸ���
typedef struct { 
    union {
        struct sockaddr        sockaddr;
        struct sockaddr_in     sockaddr_in;
#if (NGX_HAVE_INET6)
        struct sockaddr_in6    sockaddr_in6;
#endif
#if (NGX_HAVE_UNIX_DOMAIN)
        struct sockaddr_un     sockaddr_un;
#endif
        u_char                 sockaddr_data[NGX_SOCKADDRLEN];
    } u;

    socklen_t                  socklen;
    //һ�����豸listen��������bind setfib, backlog, rcvbuf, sndbuf, accept_filter, deferred, ipv6only, or so_keepalive��Щ��������bind��setһ����1
    unsigned                   set:1;

    /*
     ���ָ����default��������ô���server�齫��ͨ������ַ:�˿ڡ������з��ʵ�Ĭ�Ϸ����������������Ϊ��Щ��ƥ��server_nameָ���е�
     ������ָ��Ĭ��server������������������������ǳ����ã����û��ָ�����default��������ôĬ�Ϸ�������ʹ�õ�һ��server�顣 
     */
    unsigned                   default_server:1; //���������bind ��������ʱ��default|default_server����1����ngx_http_core_listen
/*
    instructs to make a separate bind() call for a given address:port pair. This is useful because if there are several listen 
directives with the same port but different addresses, and one of the listen directives listens on all addresses for the 
given port (*:port), nginx will bind() only to *:port. It should be noted that the getsockname() system call will be made 
in this case to determine the address that accepted the connection. If the setfib, backlog, rcvbuf, sndbuf, accept_filter, 
deferred, ipv6only, or so_keepalive parameters are used then for a given address:port pair a separate bind() call will always be made. 
*///
/*
���ж��server{}�飬�����һ��bind 1.1.1.1:1 �ڶ���server bind 2.2.2.2:2  ������server bind *:80�����������bind�����������е�����
�������ӵ����������Ӷ���ȡ�����Ӻ���Ҫʹ��getsockname���жϣ����ÿ��������bind�Ļ����Ͳ������ж��ˡ����������1.1.1.1:1�Ļᵽ��һ��bind
2.2.2.2:2�ĵ��ڶ���bind�������ĵ���������
һ�����豸listen��������bind setfib, backlog, rcvbuf, sndbuf, accept_filter, deferred, ipv6only, or so_keepalive(��Щѡ��ֻ����bind�����setsockops)��Щ��������bind��setһ����1
*/ //��ngx_http_core_listen
    unsigned                   bind:1; //listen *:80���������ͨ���Ҳ������ngx_http_init_listening����1
    unsigned                   wildcard:1;
#if (NGX_HTTP_SSL)
/*
��ָ�����������listen������������SSLģʽ���⽫���������ͬʱ������HTTP��HTTPS����Э���£����磺
    listen 80;
    listen 443 default ssl;
*/
    unsigned                   ssl:1;
#endif
#if (NGX_HTTP_SPDY)
    unsigned                   spdy:1;
#endif
#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    unsigned                   ipv6only:1;
#endif
#if (NGX_HAVE_REUSEPORT)
    unsigned                   reuseport:1; //�˿ڸ���
#endif
    unsigned                   so_keepalive:2; //listen���������so_keepalive����ʱ��1����ngx_http_core_listen ��ȡֵ1 off�ر�ȡֵ2
    unsigned                   proxy_protocol:1; //��ngx_http_core_listen 

    int                        backlog;
    int                        rcvbuf;
    int                        sndbuf;
#if (NGX_HAVE_SETFIB)
    int                        setfib;
#endif
#if (NGX_HAVE_TCP_FASTOPEN)
    int                        fastopen;
#endif
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
    int                        tcp_keepidle;
    int                        tcp_keepintvl;
    int                        tcp_keepcnt;
#endif

#if (NGX_HAVE_DEFERRED_ACCEPT && defined SO_ACCEPTFILTER)
    char                      *accept_filter;
#endif
#if (NGX_HAVE_DEFERRED_ACCEPT && defined TCP_DEFER_ACCEPT)
    ngx_uint_t                 deferred_accept;
#endif

    u_char                     addr[NGX_SOCKADDR_STRLEN + 1]; //�������˼�����IP:port�ַ�����ʽ����ngx_http_core_server
} ngx_http_listen_opt_t;

/*
    NGX_HTTP_TRY_FILES_PHASE�׶�:

    ����һ������HTTP�׶Σ�����˵�󲿷�HTTPģ�鶼���ڴ˽׶����¶���Nginx����������Ϊ�����3�����ᵽ��mytestģ�顣NGX_HTTP_CONTENT_PHASE
�׶�֮���Ա��ڶ�HTTPģ�顰�Ӱ�������Ҫ������������ԭ��
    ��һ������9���׶���Ҫרע��4�������Թ�����rewrite��дURL���ҵ�location���ÿ顢�ж������Ƿ�߱�����Ȩ�ޡ�try_files�������ȶ�ȡ��̬��Դ�ļ���
��4������ͨ�������ھ��󲿷�������ˣ����HTTPģ��ϣ�����Թ�����9���׶����Ѿ���ɵĹ��ܡ�

    �����NGX_HTTP_CONTENT_PHASE�׶��������׶ζ�����ͬ���ǣ�����HTTPģ���ṩ�����ֽ���ý׶εķ�ʽ����һ��������10���׶�һ����
ͨ����ȫ�ֵ�ngx_http_core_main_conf_t�ṹ���phases���������ngx_http_handler_pt��������ʵ�֣����ڶ����Ǳ��׶ζ��еģ���ϣ�����������
ngx_http_handler_pt�������õ�location��ص�ngx_http_core_loc_conf_t�ṹ���handlerָ���У������ǵ�3����mytest���ӵ��÷���

    ������˵�ĵ�һ�ַ�ʽ��Ҳ��HTTPģ���������10���׶ε�Ψһ��ʽ����ͨ���ڱض��ᱻ���õ�postconfiguration������ȫ�ֵ�
ngx_http_core_main_conf_t�ṹ���phases[NGX_HTTP_CONTENT_PHASE]��̬�������ngx_http_handler_pt����������ɵģ��������������Ӧ����ȫ����HTTP����

    ���ڶ��ַ�ʽ��ͨ������ngx_http_core_loc_conf_t�ṹ���handlerָ����ʵ�ֵģ�ÿһ��location����Ӧ��һ��������ngx_http_core_loc_conf��
���塣���������ǾͲ����ڱض��ᱻ���õ�postconfiguration���������ngx_http_handler_pt�������ˣ�������ѡ����ngx_command_t��ĳ��������
�����3���е�mytest������Ļص���������Ӵ�����������ǰlocation��������ngx_http_core- loc��conf_t�ṹ���е�handler����Ϊ
ngx_http_handler_pt���������������ĺô��ǣ�ngx_http_handler_pt����������Ӧ�������е�HTTP���󣬽������û������URIƥ����locationʱ
(Ҳ����mytest���������ڵ�location)�Żᱻ���á�

    ��Ҳ����ζ������һ����ȫ��ͬ�������׶ε�ʹ�÷�ʽ�� ��ˣ���HTTPģ��ʵ����ĳ��ngx_http_handler_pt��������ϣ������NGX_HTTP_CONTENT_PHASE��
���������û�����ʱ�����ϣ�����ngx_http_handler_pt����Ӧ�������е��û�������Ӧ����ngx_http_module_t�ӿڵ�postconfiguration�����У�
��ngx_http_core_main_conf_t�ṹ���phases[NGX_HTTP_CONTENT_PHASE]��̬���������ngx_http_handler_pt����������֮�����ϣ�������ʽ
��Ӧ����URIƥ�䶡ĳЩlocation���û�������Ӧ����һ��location��������Ļص������У���ngx_http_handler_pt�������õ�ngx_http_core_loc_conf_t
�ṹ���handler�С�
    ע��ngx_http_core_loc_conf_t�ṹ���н���һ��handlerָ�룬���������飬��Ҳ����ζ��������������ĵڶ��ַ������ngx_http_handler_pt��������
��ôÿ��������NGX_HTTP_CONTENT PHASE�׶�ֻ����һ��ngx_http_handler_pt����������ʹ�õ�һ�ַ���ʱ��û��������Ƶģ�NGX_HTTP_CONTENT_PHASE��
�ο��Ծ��������HTTPģ�鴦��

    ��ͬʱʹ�������ַ�ʽ����ngx_http_handler_pt������ʱ��ֻ�еڶ��ַ�ʽ���õ�ngx_http_handler_pt�������Ż���Ч��Ҳ��������
handlerָ��ķ�ʽ���ȼ����ߣ�����һ�ַ�ʽ���õ�ngx_http_handler_pt��������������Ч�����һ��location���ÿ����ж��HTTPģ���
�������ڽ������̶���ͼ���յڶ��ַ�ʽ����ngx_http_handler_pt����������ô�����������п��ܸ���ǰ������������ʱ��handlerָ������á�

NGX_HTTP_CONTENT_PHASE�׶ε�checker������ngx_http_core_content_phase��ngx_http_handler_pt�������ķ���ֵ���������ַ�ʽ�¾߱��˲�ͬ���塣
    �ڵ�һ�ַ�ʽ�£�ngx_http_handler_pt���������۷����κ�ֵ������ֱ�ӵ���ngx_http_finalize_request�����������󡣵�Ȼ��
ngx_http_finalize_request�������ݷ���ֵ�Ĳ�ͬδ�ػ�ֱ�ӽ����������ڵ�11���л���ϸ���ܡ�

    �ڵڶ��ַ�ʽ�£����ngx_http_handler_pt����������NGX_DECLINED������˳�����ִ����һ��ngx_http_handler_pt�������������������ֵ��
�����ngx_http_finalize_request������������
*/

/*
    ע��ngx_http_phases�����11���׶�����˳��ģ����밴���䶨���˳��ִ�С�ͬʱҲҪ��ʶ����������˵һ���û��������ֻ�ܾ���11��
HTTPģ���ṩ��ngx_http_handler_pt����������NGX_HTTP_POST_READ_PHASE��NGX_HTTP_SERVER_REWRITE_PHASE��NGX_HTTP_REWRITE_PHASE��
NGX_HTTP_PREACCESS_PHASE��NGX_HTTP_ACCESS_PHASE��NGX HTTP_CONTENT_PHASE��NGX_HTTP_LOG_PHASE��7���׶ο��԰�����������������
�����ǿ���ͬʱ������ͬһ���û�����ġ���NGX_HTTP_FIND_CONFIG_PHASE��NGX_HTTP_POSTREWRITE_PHASE��NGX_HTTP_POST_ACCESS_PHASE��
NGX_HTTP_TRY_FILES_PHASE��4���׶�������HTTPģ������Լ���ngx_http_handler_pt���������û��������ǽ���HTTP���ʵ�֡�
*/
typedef enum { //�����׶ε�http���check������ngx_http_init_phase_handlers
    //�ڽ��յ�������HTTPͷ�������HTTP�׶� 
    NGX_HTTP_POST_READ_PHASE = 0, //�ý׶η�����:ngx_http_realip_handler
    /*�ڻ�û�в�ѯ��URIƥ���locationǰ����ʱrewrite��дURLҲ��Ϊһ��������HTTP�׶�*/
    NGX_HTTP_SERVER_REWRITE_PHASE, //�ý׶�handler������:ngx_http_rewrite_handler

    /*����URIѰ��ƥ���location������׶�ͨ����ngx_http_core_moduleģ��ʵ�֣�����������HTTPģ�����¶�����һ�׶ε���Ϊ*/
    NGX_HTTP_FIND_CONFIG_PHASE,//�ý׶�handler������:�ޣ��������û����hander�����ڸý׶�

    //��NGX_HTTP_FIND_CONFIG_PHASE�׶�Ѱ�ҵ�ƥ���location֮�����޸������URI
    /*��NGX_HTTP_FIND_CONFIG_PHASE�׶�֮����дURL��������NGX_HTTP_SERVER_REWRITE_PHASE�׶���Ȼ�ǲ�ͬ�ģ���Ϊ�����߻ᵼ�²��ҵ���ͬ��location�飨location����URI����ƥ��ģ�*/
    NGX_HTTP_REWRITE_PHASE,//�ý׶�handler������:ngx_http_rewrite_handler
    /*��һ�׶���������rewrite��дURL����������NGX_HTTP_FIND_CONFIG_PHASE�׶Σ��ҵ����µ�URIƥ���location�����ԣ���һ�׶����޷��ɵ�����HTTPģ�鴦��ģ�������ngx_http_core_moduleģ��ʹ��*/
    /*
    ��һ�׶���������rewrite��дURL�󣬷�ֹ�����nginx��conf���õ�����ѭ�����ݹ���޸�URI������ˣ���һ�׶ν���ngx_http_core_moduleģ�鴦��
Ŀǰ��������ѭ���ķ�ʽ�ܼ򵥣����ȼ��rewrite�Ĵ��������һ�����󳬹�10���ض�������Ϊ������rewrite��ѭ������ʱ��NGX_HTTP_POSTREWRITE_PHASE
�׶ξͻ����û�����500����ʾ�������ڲ�����
     */
    NGX_HTTP_POST_REWRITE_PHASE,//�ý׶�handler������:�ޣ��������û����hander�����ڸý׶�

    //����NGX_HTTP_ACCESS_PHASE�׶�ǰ��HTTPģ����Խ���Ĵ���׶�
    NGX_HTTP_PREACCESS_PHASE,//�ý׶�handler������:ngx_http_degradation_handler  ngx_http_limit_conn_handler  ngx_http_limit_req_handler ngx_http_realip_handler

     /*����׶�������HTTPģ���ж��Ƿ���������������Nginx������*/
    NGX_HTTP_ACCESS_PHASE,//�ý׶�handler������:ngx_http_access_handler  ngx_http_auth_basic_handler  ngx_http_auth_request_handler
    /*��NGX_HTTP_ACCESS_PHASE�׶���HTTPģ���handler���������ز�������ʵĴ�����ʱ��ʵ����NGX_HTTP_FORBIDDEN����NGX_HTTP_UNAUTHORIZED����
    ����׶ν�������ܾ�������û���Ӧ�����ԣ�����׶�ʵ�������ڸ�NGX_HTTP_ACCESS_PHASE�׶���β*/
    NGX_HTTP_POST_ACCESS_PHASE,//�ý׶�handler������:�ޣ��������û����hander�����ڸý׶�

    /*����׶���ȫ��Ϊ��try_files������������ġ���HTTP������ʾ�̬�ļ���Դʱ��try_files���������ʹ�������˳��ط��ʶ����̬�ļ���Դ��
    ���ĳһ�η���ʧ�ܣ����������try_files��ָ������һ����̬��Դ�����⣬���������ȫ����NGX_HTTP_TRY_FILES_PHASE�׶���ʵ�ֵ�*/
    NGX_HTTP_TRY_FILES_PHASE,//�ý׶�handler������:�ޣ��������û����hander�����ڸý׶�

    /* ����10���׶��и�HTTPģ��Ĵ��������Ƿ���ȫ�ֵ�ngx_http_core_main_conf_t�ṹ���еģ�Ҳ����˵�����Ƕ��κ�һ��HTTP��������Ч�� 
    NGX_HTTP_CONTENT_PHASE�������ĳ������Ψһ��Ч

    ngx_http_handler_pt����������Ӧ�������е�HTTP���󣬽������û������URIƥ����locationʱ(Ҳ����mytest���������ڵ�location)�Żᱻ���á�
��Ҳ����ζ������һ����ȫ��ͬ�������׶ε�ʹ�÷�ʽ�� ��ˣ���HTTPģ��ʵ����ĳ��ngx_http_handler_pt��������ϣ������NGX_HTTP_CONTENT_PHASE��
���������û�����ʱ�����ϣ�����ngx_http_handler_pt����Ӧ�������е��û�������Ӧ����ngx_http_module_t�ӿڵ�postconfiguration�����У�
��ngx_http_core_main_conf_t�ṹ���phases[NGX_HTTP_CONTENT_PHASE]��̬���������ngx_http_handler_pt����������֮�����ϣ�������ʽ
��Ӧ����URIƥ�䶡ĳЩlocation���û�������Ӧ����һ��location��������Ļص������У���ngx_http_handler_pt�������õ�ngx_http_core_loc_conf_t
�ṹ���handler�С�
    ע��ngx_http_core_loc_conf_t�ṹ���н���һ��handlerָ�룬���������飬��Ҳ����ζ��������������ĵڶ��ַ������ngx_http_handler_pt��������
��ôÿ��������NGX_HTTP_CONTENT PHASE�׶�ֻ����һ��ngx_http_handler_pt����������ʹ�õ�һ�ַ���ʱ��û��������Ƶģ�NGX_HTTP_CONTENT_PHASE��
�ο��Ծ��������HTTPģ�鴦��
    */
    //���ڴ���HTTP�������ݵĽ׶Σ����Ǵ󲿷�HTTPģ����ϲ������Ľ׶�  //CONTENT_PHASE�׶εĴ���ص�����ngx_http_handler_pt�Ƚ����⣬��ngx_http_core_content_phase 
    NGX_HTTP_CONTENT_PHASE, //�ý׶�handler������:ngx_http_autoindex_handler  ngx_http_dav_handler ngx_http_gzip_static_handler  ngx_http_index_handler ngx_http_random_index_handler ngx_http_static_handler
    
    /*������������¼��־�Ľ׶Ρ����磬ngx_http_log_moduleģ���������׶��м�����һ��handler��������ʹ��ÿ��HTTP��������Ϻ���¼access_log��־*/
    NGX_HTTP_LOG_PHASE //�ý׶�handler������: ngx_http_log_handler
} ngx_http_phases; 

typedef struct ngx_http_phase_handler_s  ngx_http_phase_handler_t;
//һ��HTTP����׶��е�checker��鷽������������HTTP���ʵ�֣��Դ˿���HTTP����Ĵ�������
typedef ngx_int_t (*ngx_http_phase_handler_pt)(ngx_http_request_t *r,
    ngx_http_phase_handler_t *ph);

/*
ע��ͨ����������һ��ngx_http_phases�׶Σ�������ӵ���������ngx_http_phase_handler_s�ṹ�壬�京����ӽ���ĳ��HTTPģ��Ĵ�������
һ��http{}�������Ϻ󽫻����nginx.conf�е����ò�����ngx_http_phase_handler_t��ɵ����飬�ڴ���HTTP����ʱ��һ���������Щ�׶���˳��
���ִ�еģ���ngx_http_phase_handler_t�е�next��Աʹ������Ҳ���Է�˳��ִ�С�ngx_http_phase_engine_t�ṹ���������ngx_http_phase_handler_t��ɵ�����
*/    
//ע�⣺ngx_http_phase_handler_s�ṹ�����ʾ����׶��е�һ��������
struct ngx_http_phase_handler_s { //ngx_http_phase_engine_t�ṹ���������ngx_http_phase_handler_t��ɵ����飬Ҳ����ngx_http_phase_handler_s�ṹ�洢��ngx_http_phase_handler_t
    /*
    �ڴ���ĳһ��HTTP�׶�ʱ��HTTP��ܽ�����checker������ʵ�ֵ�ǰ�������ȵ���checker�������������󣬶�����ֱ�ӵ����κν׶��е�handler��
����ֻ����checker�����вŻ�ȥ����handler��������ˣ���ʵ�����е�checker���������ɿ���е�ngx_http_core moduleģ��ʵ�ֵģ�����ͨ��HTTPģ����
���ض���checker����
     */
/*  HTTP���Ϊ11���׶�ʵ�ֵ�checker����  ��ֵ��ngx_http_init_phase_handlers  //���н׶ε�checker��ngx_http_core_run_phases�е���
���������������������������������ש�����������������������������������
��    �׶�����                  ��    checker����                   ��
���������������������������������ש�����������������������������������
��   NGX_HTTP_POST_READ_PHASE   ��    ngx_http_core_generic_phase   ��
�ǩ������������������������������贈����������������������������������
��NGX HTTP SERVER REWRITE PHASE ��ngx_http_core_rewrite_phase       ��
�ǩ������������������������������贈����������������������������������
��NGX HTTP FIND CONFIG PHASE    ��ngx_http_core find config_phase   ��
�ǩ������������������������������贈����������������������������������
��NGX HTTP REWRITE PHASE        ��ngx_http_core_rewrite_phase       ��
�ǩ������������������������������贈����������������������������������
��NGX HTTP POST REWRITE PHASE   ��ngx_http_core_post_rewrite_phase  ��
�ǩ������������������������������贈����������������������������������
��NGX HTTP PREACCESS PHASE      ��ngx_http_core_generic_phase       ��
�ǩ������������������������������贈����������������������������������
��NGX HTTP ACCESS PHASE         ��ngx_http_core_access_phase        ��
�ǩ������������������������������贈����������������������������������
��NGX HTTP POST ACCESS PHASE    ��ngx_http_core_post_access_phase   ��
�ǩ������������������������������贈����������������������������������
��NGX HTTP TRY FILES PHASE      ��ngx_http_core_try_files_phase     ��
�ǩ������������������������������贈����������������������������������
��NGX HTTP CONTENT PHASE        ��ngx_http_core_content_phase       ��
�ǩ������������������������������贈����������������������������������
��NGX HTTP LOG PHASE            ��ngx_http_core_generic_phase       ��
���������������������������������ߩ�����������������������������������
*/
     //����ͬһngx_http_phases�׶ε�����ngx_http_phase_handler_t��checkerָ����ͬ�ĺ�������ngx_http_init_phase_handlers
     //ngx_http_phases�׶��е�ÿһ���׶ζ��ж�Ӧ��checker������ͨ����checker������ִ�и��Զ�Ӧ�ġ���checker������ngx_http_core_run_phases��ִ��
    ngx_http_phase_handler_pt  checker; //�����׶εĳ�ʼ����ֵ��ngx_http_init_phase_handlers�е�checker������ִ�и��Ե�handler����,cheker��http��ܺ�����handler�Ƕ�Ӧ���û�����ģ�麯��
//��ngx_http_core moduleģ�������HTTPģ�飬ֻ��ͨ������handler�������ܽ���ĳһ��HTTP����׶��Դ�������
    ngx_http_handler_pt        handler; //ֻ����checker�����вŻ�ȥ����handler����,��ngx_http_core_run_phases
/*
��Ҫִ�е���һ��HTTP����׶ε����
next�����ʹ�ô���׶β��ذ�˳������ִ�У��ȿ��������Ծ�����׶μ���ִ�У�Ҳ������Ծ��֮ǰ����ִ�й���ĳ���׶�����ִ�С�ͨ����
next��ʾ��һ������׶��еĵ�1��ngx_http_phase_handler_s������
*/
    ngx_uint_t                 next;//��һ�׶εĵ�һ��ngx_http_handler_pt�������������е�λ��
};


/*
ע��ͨ����������һ��ngx_http_phases�׶Σ�������ӵ���������ngx_http_phase_handler_s�ṹ�壬�京����ӽ���ĳ��HTTPģ��Ĵ�������
һ��http{}�������Ϻ󽫻����nginx.conf�е����ò�����ngx_http_phase_handler_t��ɵ����飬�ڴ���HTTP����ʱ��һ���������Щ�׶���˳��
���ִ�еģ���ngx_http_phase_handler_t�е�next��Աʹ������Ҳ���Է�˳��ִ�С�ngx_http_phase_engine_t�ṹ���������ngx_http_phase_handler_t��ɵ�����

ngx_http_phase_engine_t�б������ڵ�ǰnginx.conf�����£�һ���û�������ܾ���������ngx_http_handler_pt����������������HTTPģ����Ժ��������û�����
�Ĺؼ������ngx_http_phase_engine_t�ṹ���Ǳ�����ȫ�ֵ�ngx_http_core_main_conf_t�ṹ���е�

��HTTP��ܵĳ�ʼ�������У��κ�HTTPģ�鶼������ngx_http_module_t�ӿڵ�postconfiguration�����н��Զ���ķ�����ӵ�handler��̬�����У���������������ͻ���
����ӵ�ngx_http_core_main_conf_t->phase_engine��
*/  
//�ռ䴴������ֵ�ο�ngx_http_init_phase_handlers
typedef struct { //ngx_http_phase_engine_t�ṹ���Ǳ�����ȫ�ֵ�ngx_http_core_main_conf_t�ṹ���е�
    /* handlers����ngx_http_phase_handler_t���ɵ������׵�ַ������ʾһ��������ܾ���������ngx_http_handler_pt��������
    ���ngx_http_request_t�ṹ���е�phase_handler��Աʹ�ã�phase_handlerָ���˵�ǰ����Ӧ��ִ����һ��HTTP�׶Σ�*/
    ngx_http_phase_handler_t  *handlers;
    
    /* ��ʾNGX_HTTP_SERVER_REWRITE_PHASE�׶ε�1��ngx_http_phase_handler_t��������handlers�����е���ţ�������ִ��HTTP����
    ���κν׶��п�����ת��NGX_HTTP_SERVER_REWRITE_PHASE�׶δ������� */
    ngx_uint_t                 server_rewrite_index; //��ֵ�ο�ngx_http_init_phase_handlers
    /*
    ��ʾNGX_HTTP_REWRITE_PHASE�׶ε�1��ngx_http_phase_handler_t��������handlers�����е���ţ�������ִ��HTTP������κν׶���
    ������ת��NGX_HTTP_REWRITE_PHASE�׶δ�������
     */
    ngx_uint_t                 location_rewrite_index; //��ֵ�ο�ngx_http_init_phase_handlers
} ngx_http_phase_engine_t;


typedef struct { //�洢��ngx_http_core_main_conf_t->phases[]
    //handlers��̬���鱣����ÿһ��HTTPģ���ʼ��ʱ��ӵ���ǰ�׶εĴ�����
    ngx_array_t                handlers; //�����д洢����ngx_http_handler_pt
} ngx_http_phase_t;

//�ο�:http://tech.uc.cn/?p=300
typedef struct {//��ʼ����ֵ�ο�ngx_http_core_module_ctx
/*
servers��̬�����е�ÿһ��Ԫ�ض���һ��ָ�룬��ָ�����ڱ�ʾserver���ngx_http_core_srv_conf_t�ṹ��ĵ�ַ������ngx_http_core_moduleģ�飩��
ngx_http_core_srv_conf_t�ṹ������1��ctxָ�룬��ָ�����server��ʱ�����ɵ�ngx_http_conf_ctx_t�ṹ��,���ֻҪ��ȡ��http{}��������ctx�������ҵ�http{}
��server{}���������ctx��ͼ�λ����ο�ͼ10-3
 */
    /* 
     http {
        server {
            xxx;
        }   

        server {
            xxx;
        }
     } */ //servers����洢���ǵ�������http{}��server{}�е�ʱ�򣬸�server��Ӧ��ngx_http_core_srv_conf_t,�ο�ngx_http_core_server�е�ngx_array_push����������Կ���
    ngx_array_t                servers;  /* ngx_http_core_srv_conf_t */ //��ngx_http_core_create_main_conf�д���server��

/*
��HTTP��ܵĳ�ʼ�������У��κ�HTTPģ�鶼������ngx_http_module_t�ӿڵ�postconfiguration�����н��Զ���ķ�����ӵ�ngx_http_phase_t->handler��̬�����У�
��������������ͻ�������ӵ�ngx_http_core_main_conf_t->phase_engine��
 */
    //��������׶δ��������ɵ�phases���鹹���Ľ׶����������ˮʽ����HTTP�����ʵ�����ݽṹ
    ngx_http_phase_engine_t    phase_engine; //��ngx_http_core_main_conf_t�й���HTTP�׶���������Ա��phase_engine��phases
    //ngx_http_headers_in�е�ȫ����Ա���ڸ�hash���У���ngx_http_init_headers_in_hash
    ngx_hash_t                 headers_in_hash;//��ngx_http_headers_in  ngx_http_init_headers_in_hash

    ngx_hash_t                 variables_hash;

    ngx_array_t                variables;       /* ngx_http_variable_t */ 
    ngx_uint_t                 ncaptures;

    ngx_uint_t                 server_names_hash_max_size;
    ngx_uint_t                 server_names_hash_bucket_size;

    ngx_uint_t                 variables_hash_max_size; //Ĭ��ֵ��ngx_http_core_init_main_conf
    ngx_uint_t                 variables_hash_bucket_size; //Ĭ��ֵ��ngx_http_core_init_main_conf

    ngx_hash_keys_arrays_t    *variables_keys;

    //�����Ǹ������ԭ����:�����������listen 1.1.1.1:50  2.2.2.2:50,��˿ڶ���50��������IP��һ�������Ǵ洢�ڸ������У�����˿�һ��������˿ں�IP��ַһ��������Ե�һ��Ϊ׼����һ��
    //����뵽ͬһ��ngx_http_conf_port_t�ڵ��У�����������˿ڵ�ַ����ŵ���һ��ngx_http_conf_port_t�ڵ��У���ͬ�˿ڴ洢�ڸýṹ��Ĳ�ͬ�ڵ�
    //����http�е�listen�������ڲ�ͬserver{]�ֵ�listen,���ǵ�listenͷ��ӵ�ngx_http_core_main_conf_t->ports,��ngx_http_add_listen
    //��addrs���򣬴�ͨ����ĵ�ַ���ں��棬 (listen 1.2.2.2:30 bind) > listen 1.1.1.1:30  > listen *:30,��ngx_http_block
    ngx_array_t               *ports;//û������һ��listen����������һ��ngx_http_conf_port_t  ��ֵ��ngx_http_add_listen���洢����ngx_http_conf_port_t�ṹ

    ngx_uint_t                 try_files;       /* unsigned  try_files:1 */

/*
��ngx_http_core_main_conf_t�й���HTTP�׶���������Ա��phase_engine��phases������phase_engine�������й�����һ��HTTP������Ҫ
������HTTP����׶Σ��������ngx_http_request_t�ṹ���е�phase_handler��Աʹ�ã�phase_handlerָ���˵�ǰ����Ӧ��ִ����һ��HTTP�׶Σ���
��phases�������һ����ʱ��������ʵ���Ͻ�����Nginx�����������õ�������Ψһʹ���ǰ���11���׶εĸ����ʼ��phase_engine�е�handlers����
 */
/*
������HTTP��ܳ�ʼ��ʱ��������HTTPģ��������׶������HTTP������������һ����11����Ա��ngx_http_phase_t���飬����ÿһ��ngx_http_phase_t
�ṹ���Ӧһ��HTTP�׶Ρ���HTTP��ܳ�ʼ����Ϻ����й����е�phases���������õ�

 NGX_HTTP_FIND_CONFIG_PHASE��NGX_HTTP_POSTREWRITE_PHASE��NGX_HTTP_POST_ACCESS_PHASE�� NGX_HTTP_TRY_FILES_PHASE��4���׶�����
 ��HTTPģ������Լ���ngx_http_handler_pt���������û��������ǽ���HTTP���ʵ�֡�����7���׶��������Ǽ���ngx_http_handler_pt����
��������뵽phases[]�����׶ε�ngx_http_handler_pt����������������֣��������ngx_http_phase_engine_t->handlers������Ҳ����������ĸ��׶εĴ�����
 */ //phases�������һ����ʱ��������ʵ���Ͻ�����Nginx�����������õ�������Ψһʹ���ǰ���11���׶εĸ����ʼ��phase_engine�е�handlers����
    //��ngx_http_block����ִ��ÿ��ģ��module->postconfiguration�ӿڵ�ʱ����phases�������ngx_http_handler_pt��������������Բο�����ngx_http_rewrite_init��
    //�ռ䴴���ͳ�ʼ����ngx_http_init_phases
    ngx_http_phase_t           phases[NGX_HTTP_LOG_PHASE + 1]; 
} ngx_http_core_main_conf_t;

/*
= ��ͷ��ʾ��ȷƥ��
^~ ��ͷ��ʾuri��ĳ�������ַ�����ͷ�����Ϊƥ�� url·�����ɡ�nginx����url�����룬�������Ϊ/static/20%/aa�����Ա�����^~ /static/ /aaƥ�䵽��ע���ǿո񣩡�
~ ��ͷ��ʾ���ִ�Сд������ƥ��
~*  ��ͷ��ʾ�����ִ�Сд������ƥ��
!~��!~*�ֱ�Ϊ���ִ�Сд��ƥ�估�����ִ�Сд��ƥ�� ������
/ ͨ��ƥ�䣬�κ����󶼻�ƥ�䵽��


locationƥ������

~      #�����߱�ʾִ��һ������ƥ�䣬���ִ�Сд
~*    #��ʾִ��һ������ƥ�䣬�����ִ�Сд
^~    #^~��ʾ��ͨ�ַ�ƥ�䣬�����ѡ��ƥ�䣬ֻƥ���ѡ���ƥ����ѡ�һ������ƥ��Ŀ¼
=      #������ͨ�ַ���ȷƥ��
@     #"@" ����һ�������� location��ʹ�����ڲ�����ʱ������ error_page, try_files



location ƥ������ȼ�(��location�������ļ��е�˳���޹�)
= ��ȷƥ����һ��������������־�ȷƥ�䣬nginxֹͣ��������ƥ�䡣
��ͨ�ַ�ƥ�䣬������ʽ����ͳ��Ŀ���򽫱����ȺͲ�ѯƥ�䣬Ҳ����˵�������ƥ�仹��ȥ����û��������ʽƥ��͸�����ƥ�䡣
^~ ��ֻƥ��ù���nginxֹͣ��������ƥ�䣬����nginx�������������locationָ�
���ƥ�������"~"��"~*"��ָ�����ҵ���Ӧ��ƥ�䣬��nginxֹͣ��������ƥ�䣻��û��������ʽ����û��������ʽ��ƥ�������£���ôƥ��̶���ߵ�����ƥ��ָ��ᱻʹ�á�

location ���ȼ��ٷ��ĵ�

1.Directives with the = prefix that match the query exactly. If found, searching stops.
2.All remaining directives with conventional strings, longest match first. If this match used the ^~ prefix, searching stops.
3.Regular expressions, in order of definition in the configuration file.
4.If #3 yielded a match, that result is used. Else the match from #2 is used.
1.=ǰ׺��ָ���ϸ�ƥ�������ѯ������ҵ���ֹͣ������
2.����ʣ�µĳ����ַ��������ƥ�䡣������ƥ��ʹ��^?ǰ׺������ֹͣ��
3.������ʽ���������ļ��ж����˳��
4.�����3���������ƥ��Ļ��������ʹ�á�������ͬ�ӵ�2������ʹ�á�


����

location  = / {
# ֻƥ��"/".
[ configuration A ] 
}
location  / {
# ƥ���κ�������Ϊ������������"/"��ʼ
# ���Ǹ����ַ�ƥ�����������ʽƥ�������ƥ��
[ configuration B ] 
}
location ^~ /images/ {
# ƥ���κ��� /images/ ��ʼ�����󣬲�ֹͣƥ�� ����location
[ configuration C ] 
}
location ~* \.(gif|jpg|jpeg)$ {
# ƥ���� gif, jpg, or jpeg��β������. 
# �������� /images/ Ŀ¼�������� [Configuration C]����.   
[ configuration D ] 
}����URI����:

?/ -> ����configuration A
?/documents/document.html -> ����configuration B
?/images/1.gif -> ����configuration C
?/documents/1.jpg ->���� configuration D
@location ����
error_page 404 = @fetch;

location @fetch(
proxy_pass http://fetch;
)
*/

typedef struct {
    /*
�������ü���socket��ָ����Ҫ��������server_name��listen��server_nameָ������ʵ�����������Ĺ��ܣ�������ÿ��server���������������
�ڴ�������ʱ������������е�host��ת������,listen�������մ����ngx_http_core_main_conf_t->ports
��ǰserver���������������������ڵĻ��������HTTP�����е�Hostͷ����ƥ�䣬ƥ���Ϻ����ɵ�ǰngx_http_core_srv_conf_t�������� */
    /*   
     server {
         listen       80;
         server_name  example.org  www.example.org;
         ...
     }
     */ /* array of the ngx_http_server_name_t, "server_name" directive */
    ngx_array_t                 server_names;//���Բο�ngx_http_core_server_name

    /* server ctx */
    /*
    ָ��ǰserver{}�п���ngx_http_conf_ctx_t�ռ�������ģ��ο�ngx_http_core_server�� ngx_http_core_srv_conf_t�ṹ�ռ䶼����
    ngx_http_conf_ctx_t�е�srv_conf����
    */ //ָ��ǰserver��������ngx_http_conf_ctx_t�ṹ��
    ngx_http_conf_ctx_t        *ctx; //ִ�ж�Ӧ��server{}������ʱ�򿪱ٵ�ngx_http_conf_ctx_t

    ngx_str_t                   server_name; 

    size_t                      connection_pool_size; //Ĭ��256
    size_t                      request_pool_size; //Ĭ��4096����ngx_http_core_merge_srv_conf
    size_t                      client_header_buffer_size;

    //client_header_timeoutΪ��ȡ�ͻ�������ʱĬ�Ϸ���Ŀռ䣬����ÿռ䲻���洢httpͷ���к������У�������large_client_header_buffers
    //���·���ռ䣬����֮ǰ�Ŀռ����ݿ������¿ռ��У����ԣ�����ζ�ſɱ䳤�ȵ�HTTP�����м���HTTPͷ���ĳ����ܺͲ��ܳ���large_client_ header_
    //buffersָ�����ֽ���������Nginx���ᱨ��
    ngx_bufs_t                  large_client_header_buffers; 
    //timer_resolution����������Ͽ��Ա�֤��ʱ��ÿ����ô�����ж�һ�Σ��Ӷ����Դ�epoll�з��أ�������ʱ�䣬�ж���Щ�¼��г�ʱ��ִ�г�ʱ�¼�������ͻ��˼��ϴ�
    //���������������client_header_timeoutʱ���û����������������ر�����
    //ע�⣬�ڽ�����������ͷ���к������к󣬻���ngx_http_process_request�л�Ѷ��¼���ʱ��ʱ��ɾ��
    ngx_msec_t                  client_header_timeout; //Ĭ��60�룬��������õĻ�      ע��������large_client_header_buffers��Ͻ���

    ngx_flag_t                  ignore_invalid_headers;
    ngx_flag_t                  merge_slashes;
    ngx_flag_t                  underscores_in_headers; //HTTPͷ���Ƿ������»���, ��ngx_http_parse_header_line

    unsigned                    listen:1;
#if (NGX_PCRE)
    unsigned                    captures:1;
#endif

    ngx_http_core_loc_conf_t  **named_locations; //ָ��server{}������location�����е�����name  location @name,��ngx_http_init_locations
} ngx_http_core_srv_conf_t;


/* list of structures to find core_srv_conf quickly at run time */

//server_name������������Ϣ����ngx_http_core_server_name
typedef struct {
#if (NGX_PCRE)
    ngx_http_regex_t          *regex;
#endif
    //server_name������������server{}����ngx_http_core_server_name
    ngx_http_core_srv_conf_t  *server;   /* virtual name server conf */
    ngx_str_t                  name; //server_name xxx, name����xxx
} ngx_http_server_name_t;

//�ṹngx_http_virtual_names_t�еĳ�Ա
typedef struct { //�����ռ�͸�ֵ��ngx_http_add_addrs
     ngx_hash_combined_t       names;

     ngx_uint_t                nregex;
     ngx_http_server_name_t   *regex;
} ngx_http_virtual_names_t;

//���http{}����δ��ȷ���õ�listen(bind = 0)�д���ͨ���listen(listen *:0)����Щδ��ȷ�������ngx_http_addr_conf_s��Աȡֵ����ͨ���������
//��Ӧ��ngx_http_conf_addr_t�е���س�Ա����ngx_http_add_addrs
//ngx_http_port_t->ngx_http_in_addr_t->ngx_http_addr_conf_s
//������洢��server_name������Ϣ�Լ���ip:port��Ӧ����������Ϣ
struct ngx_http_addr_conf_s {//�����ռ丳ֵ��ngx_http_add_addrs�� �ýṹ����ngx_http_in_addr_t�е�conf��Ա
    /* the default server configuration for this address:port */
    ngx_http_core_srv_conf_t  *default_server;

    ngx_http_virtual_names_t  *virtual_names; //�����ռ丳ֵ��ngx_http_add_addrs

#if (NGX_HTTP_SSL)
    unsigned                   ssl:1;
#endif
#if (NGX_HTTP_SPDY)
    unsigned                   spdy:1;
#endif
    unsigned                   proxy_protocol:1;
};

//ngx_http_port_t->ngx_http_in_addr_t->ngx_http_addr_conf_s
typedef struct {//�ýṹ�����ngx_http_port_t
    in_addr_t                  addr; //��listen�ĵ�ַ�� = sin->sin_addr.s_addr;��ngx_http_add_addrs

    //���http{}����δ��ȷ���õ�listen(bind = 0)�д���ͨ���listen(listen *:0)����Щδ��ȷ�������ngx_http_addr_conf_s��Աȡֵ����ͨ���������
//��Ӧ��ngx_http_conf_addr_t�е���س�Ա����ngx_http_add_addrs
    ngx_http_addr_conf_t       conf;//������洢��server_name������Ϣ�Լ���ip:port��Ӧ����������Ϣ
} ngx_http_in_addr_t;//�����ռ丳ֵ��ngx_http_add_addrs


#if (NGX_HAVE_INET6)

typedef struct {
    struct in6_addr            addr6;
    ngx_http_addr_conf_t       conf;
} ngx_http_in6_addr_t;

#endif

//ngx_http_port_t->ngx_http_in_addr_t->ngx_http_addr_conf_s
//��ngx_http_add_addrs 
//ngx_connection_t->listening->serversָ��ýṹ����ngx_http_init_connection       
typedef struct {//�ýṹ������ngx_conf_t->cycle->listening->servers�洢�ÿռ䣬��ngx_http_init_listening
    /* ngx_http_in_addr_t or ngx_http_in6_addr_t */
    void                      *addrs; //�����ռ���ngx_http_add_addrs
    ngx_uint_t                 naddrs; //�����¼������������ͬport�к���ͨ����Լ����ڷ�bind�����õĸ�������ֵ��ngx_http_init_listening�� 
} ngx_http_port_t; //�������ֵ���Ǵ�ngx_http_conf_addr_t�����ȡ����

/*
ngx_http_core_main_conf_t

    |---> prots�� �����Ķ˿ںŵ�����

                |---> ngx_http_conf_port_t���˿ںŵ�������Ϣ

                               |---> addrs���ڸö˿ں��ϣ����������е�ַ������ ��addrs�µ����ж˿���ͬ

                                            |---> ngx_http_conf_addr_t����ַ������Ϣ�������ڸ�addr:port�ϵĶ����������

                                                           |---> servers����addr:port�ϵ�˵��server���������Ϣngx_http_core_srv_conf_t(//���粻ͬserver{}������ͬ��listen ip:port�����Ƕ���ͬһ��ngx_http_conf_addr_t�У���serversָ��ͬ)

                                                           |            |---> ngx_http_core_srv_conf_t

                                                           |---> opt��ngx_http_listen_opt_t���ͣ�����socket��������Ϣ

                                                           |---> hash����server_nameΪkey��ngx_http_core_srv_conf_tΪvalue��hash������server_name����ͨ�����

                                                           |---> wc_head��ͬhash��server_name��ǰ׺ͨ�����

                                                           |---> wc_tail��ͬhash��server_name����׺ͨ�����



*/
//����http�е�listen�������ڲ�ͬserver{]�ֵ�listen,���ǵ�listenͷ��ӵ�ngx_http_core_main_conf_t->ports,��ngx_http_add_listen
//�ýṹ�洢��ngx_http_core_main_conf_t�е�ports���������洢listen������������Ϣ
//����listen�Ķ˿���ͬ��IP��ͬ��listen��Ϣȫ���洢�ڸýṹ�У���ͬ��ip��ַ����addrs�����С������˿�������Ϣ��addrs���ڸö˿������м�����ַ�����顣
typedef struct { //��ngx_http_add_listen
    ngx_int_t                  family; //Э����
    in_port_t                  port; //listen�������ü����Ķ˿�
//����ÿһ���˿���Ϣ��ngx_http_conf_port_t��,������һ���ֶ�Ϊaddrs������ֶ���һ�����飬��������ڴ�ŵ�ȫ�ǵ�ַ��Ϣ��ngx_http_conf_addr_t����һ����ַ��Ϣ
//��ngx_http_conf_addr_t����Ӧ�Ŷ��server{}���ÿ飨ngx_http_core_srv_conf_t��
    //��ͬport��ͬIP����ô����������֣������в�ͬ��ngx_http_conf_addr_t��Ϣ���������server{}����ͬ��listen ip:port,��ڶ�����������listen�ᱻ����
    ngx_array_t                addrs;     /* array of ngx_http_conf_addr_t */  //��ngx_http_add_address����ռ�     ��ͬIP�Ͷ˿ڵ�ֻ������һ�����Ե�һ��Ϊ׼
} ngx_http_conf_port_t; //

/*
ngx_http_core_main_conf_t

    |---> prots�� �����Ķ˿ںŵ�����

                |---> ngx_http_conf_port_t���˿ںŵ�������Ϣ

                               |---> addrs���ڸö˿ں��ϣ����������е�ַ�����飬��addrs�µ����ж˿���ͬ  ��addrs�µ����ж˿���ͬ

                                            |---> ngx_http_conf_addr_t����ַ������Ϣ�������ڸ�addr:port�ϵĶ����������

                                                           |---> servers����addr:port�ϵ�˵��server���������Ϣngx_http_core_srv_conf_t(//���粻ͬserver{}������ͬ��listen ip:port�����Ƕ���ͬһ��ngx_http_conf_addr_t�У���serversָ��ͬ)

                                                           |            |---> ngx_http_core_srv_conf_t

                                                           |---> opt��ngx_http_listen_opt_t���ͣ�����socket��������Ϣ

                                                           |---> hash����server_nameΪkey��ngx_http_core_srv_conf_tΪvalue��hash������server_name����ͨ�����

                                                           |---> wc_head��ͬhash��server_name��ǰ׺ͨ�����

                                                           |---> wc_tail��ͬhash��server_name����׺ͨ�����
*/
//listen������������洢�ڸýṹ�У�Ȼ�������ȫ���ŵ�ngx_http_core_main_conf_t�е�ports
//�ýṹ�洢��ngx_http_conf_port_t�е�addrs��
/*
������ַ������Ϣ�������������ڸ�addr:port����������server���ngx_http_core_srv_conf_t�ṹ���Լ�hash��wc_head��wc_tail��Щhash�ṹ��
��������server nameΪkey��ngx_http_core_srv_conf_tΪvalue�Ĺ�ϣ�����ڿ��ٲ��Ҷ�Ӧ����������������Ϣ��
*/
typedef struct { //��ֵ��ngx_http_add_address
    //ָ��"listen"��������������Ϣ�������ͬlisten ip:port�����ڲ�ͬ��server�У���ôoptָ����������listen�ṹngx_http_listen_opt_t����ngx_http_add_addresses
    ngx_http_listen_opt_t      opt; 

    //������������ֵ��ngx_http_server_names�������ǽ���server_name�ַ�������hash��Ĵ洢��ַ
    //��������Ա��¼����listen ip:PORT�����������ڵ�server{}�е�server_name������Ϣ
    ngx_hash_t                 hash; //��server_nameΪkey��ngx_http_core_srv_conf_tΪvalue��hash������server_name����ͨ�����
    ngx_hash_wildcard_t       *wc_head; //ͬhash��server_name��ǰ׺ͨ�����
    ngx_hash_wildcard_t       *wc_tail; //ͬhash��server_name����׺ͨ�����

#if (NGX_PCRE)
    ngx_uint_t                 nregex;
    ngx_http_server_name_t    *regex; //������ʽ��server_name���������� ��ngx_http_server_names
#endif

    /* the default server configuration for this address:port */
    //��ͬlisten ip:port�����ڲ�ͬ��server�У���ôoptָ����������listen�����д���default_serverѡ������Ӧ��server{}������ctx����ngx_http_add_addresses
    //Ĭ�ϳ�ʼ����ʱ��ֱ��ָ��listen ip:port����server{}

    /* ���listen ip:port��Ψһ��ip:port����ָ���Լ���server{}�����ģ�����Ǵ�����ͬ��listen ip:port����
    �ڲ�ͬ��server{}�У���default_serverָ��listen ip:port�����д���default_server��������server{}������ngx_http_core_srv_conf_t */
    ngx_http_core_srv_conf_t  *default_server;  
    //�洢���Ǹ�listen������������server{}��Ӧ��ngx_http_core_srv_conf_t����ֵ��ngx_http_add_server
    //���粻ͬserver{}������ͬ��listen ip:port�����Ƕ���ͬһ��ngx_http_conf_addr_t�У���serversָ����Բ�ͬ��ngx_http_core_srv_conf_t�洢�ڸ�servers������

    /* ���listen ip:port��Ψһ��ip:port����ָ���Լ���server{}�����ģ�����Ǵ�����ͬ��listen ip:port����
    �ڲ�ͬ��server{}�У���ÿ��ip:port��Ӧ��server{}�����Ĵ洢�ڸ�servers�����У���ngx_http_add_addresses */
    ngx_array_t                servers;  /* array of ngx_http_core_srv_conf_t */  
} ngx_http_conf_addr_t;

typedef struct {
    ngx_int_t                  status;
    ngx_int_t                  overwrite;
    ngx_http_complex_value_t   value;
    ngx_str_t                  args;
} ngx_http_err_page_t;


typedef struct {
    ngx_array_t               *lengths;
    ngx_array_t               *values;
    ngx_str_t                  name;

    unsigned                   code:10;
    unsigned                   test_dir:1;
} ngx_http_try_file_t;

/*
= ��ͷ��ʾ��ȷƥ��
^~ ��ͷ��ʾuri��ĳ�������ַ�����ͷ�����Ϊƥ�� url·�����ɡ�nginx����url�����룬�������Ϊ/static/20%/aa�����Ա�����^~ /static/ /aaƥ�䵽��ע���ǿո񣩡�
~ ��ͷ��ʾ���ִ�Сд������ƥ��
~*  ��ͷ��ʾ�����ִ�Сд������ƥ��
!~��!~*�ֱ�Ϊ���ִ�Сд��ƥ�估�����ִ�Сд��ƥ�� ������
/ ͨ��ƥ�䣬�κ����󶼻�ƥ�䵽��


locationƥ������

~      #�����߱�ʾִ��һ������ƥ�䣬���ִ�Сд
~*    #��ʾִ��һ������ƥ�䣬�����ִ�Сд
^~    #^~��ʾ��ͨ�ַ�ƥ�䣬�����ѡ��ƥ�䣬ֻƥ���ѡ���ƥ����ѡ�һ������ƥ��Ŀ¼
=      #������ͨ�ַ���ȷƥ��
@     #"@" ����һ�������� location��ʹ�����ڲ�����ʱ������ error_page, try_files



location ƥ������ȼ�(��location�������ļ��е�˳���޹�)
= ��ȷƥ����һ��������������־�ȷƥ�䣬nginxֹͣ��������ƥ�䡣
��ͨ�ַ�ƥ�䣬������ʽ����ͳ��Ŀ���򽫱����ȺͲ�ѯƥ�䣬Ҳ����˵�������ƥ�仹��ȥ����û��������ʽƥ��͸�����ƥ�䡣
^~ ��ֻƥ��ù���nginxֹͣ��������ƥ�䣬����nginx�������������locationָ�
���ƥ�������"~"��"~*"��ָ�����ҵ���Ӧ��ƥ�䣬��nginxֹͣ��������ƥ�䣻��û��������ʽ����û��������ʽ��ƥ�������£���ôƥ��̶���ߵ�����ƥ��ָ��ᱻʹ�á�

location ���ȼ��ٷ��ĵ�

1.Directives with the = prefix that match the query exactly. If found, searching stops.
2.All remaining directives with conventional strings, longest match first. If this match used the ^~ prefix, searching stops.
3.Regular expressions, in order of definition in the configuration file.
4.If #3 yielded a match, that result is used. Else the match from #2 is used.
1.=ǰ׺��ָ���ϸ�ƥ�������ѯ������ҵ���ֹͣ������
2.����ʣ�µĳ����ַ��������ƥ�䡣������ƥ��ʹ��^?ǰ׺������ֹͣ��
3.������ʽ���������ļ��ж����˳��
4.�����3���������ƥ��Ļ��������ʹ�á�������ͬ�ӵ�2������ʹ�á�


����

location  = / {
# ֻƥ��"/".
[ configuration A ] 
}
location  / {
# ƥ���κ�������Ϊ������������"/"��ʼ
# ���Ǹ����ַ�ƥ�����������ʽƥ�������ƥ��
[ configuration B ] 
}
location ^~ /images/ {
# ƥ���κ��� /images/ ��ʼ�����󣬲�ֹͣƥ�� ����location
[ configuration C ] 
}
location ~* \.(gif|jpg|jpeg)$ {
# ƥ���� gif, jpg, or jpeg��β������. 
# �������� /images/ Ŀ¼�������� [Configuration C]����.   
[ configuration D ] 
}����URI����:

?/ -> ����configuration A
?/documents/document.html -> ����configuration B
?/images/1.gif -> ����configuration C
?/documents/1.jpg ->���� configuration D
@location ����
error_page 404 = @fetch;

location @fetch(
proxy_pass http://fetch;
)
*/
//��ֹ��̼�ngx_http_init_locations
//�ο�ngx_http_core_location
struct ngx_http_core_loc_conf_s {
    ngx_str_t     name;          /* location name */ //location������ĵ�uri�ַ���

#if (NGX_PCRE)
    ngx_http_regex_t  *regex; //��NULL����ʾΪ����ƥ��
#endif
    //"if"���� ���� limit_except������λ
    unsigned      noname:1;   /* "if () {}" block or limit_except */ //nginx���if ָ������Ҳ����һ��location����noname���͡�
    
    unsigned      lmt_excpt:1; //limit_except������λ
 // @  ��ʾΪһ��location�������������Զ���һ��location�����location���ܱ���������ʣ�ֻ������Nginx��������������ҪΪerror_page��try_files��  
    unsigned      named:1; //�ԡ�@����ͷ������location����location @test {}
    

    unsigned      exact_match:1; //���� location = / {}����ν׼ȷƥ�䡣
    unsigned      noregex:1; //û������ָ����location ^~ /a { ... } ��location��  ǰ׺ƥ��
    

    unsigned      auto_redirect:1;
#if (NGX_HTTP_GZIP)
    unsigned      gzip_disable_msie6:2;
#if (NGX_HTTP_DEGRADATION)
    unsigned      gzip_disable_degradation:2;
#endif
#endif

    ngx_http_location_tree_node_t   *static_locations; //��ngx_http_init_static_location_trees�ж�exact/inclusive/noregex������������
#if (NGX_PCRE)
    ngx_http_core_loc_conf_t       **regex_locations; /* ���е�location ������ʽ {}����ngx_http_core_loc_conf_tȫ��ָ��regex_locations */
#endif

    /* pointer to the modules' loc_conf */
    //ִ��location{} ctx��ctx->loc_conf
    void        **loc_conf; //��ֵ��ngx_http_core_location��ָ��ngx_http_conf_ctx_t->loc_conf

    uint32_t      limit_except;
    void        **limit_except_loc_conf;

    //��mytestΪ����ģ��ע���handler����ngx_http_core_content_phaseִ�еģ���ʵ��ngx_http_finalize_request������ngx_http_mytest_handler��������á�
    //�ú�����ngx_http_core_content_phase�е�ngx_http_finalize_request(r, r->content_handler(r));�����r->content_handler(r)ִ��
    //��ngx_http_update_location_config�и�ֵ��r->content_handler = clcf->handler;

    /*
    ngx_http_handler_pt����������Ӧ�������е�HTTP���󣬽������û������URIƥ����locationʱ(Ҳ����mytest���������ڵ�location)�Żᱻ���á�
��Ҳ����ζ������һ����ȫ��ͬ�������׶ε�ʹ�÷�ʽ�� ��ˣ���HTTPģ��ʵ����ĳ��ngx_http_handler_pt��������ϣ������NGX_HTTP_CONTENT_PHASE��
���������û�����ʱ�����ϣ�����ngx_http_handler_pt����Ӧ�������е��û�������Ӧ����ngx_http_module_t�ӿڵ�postconfiguration�����У�
��ngx_http_core_main_conf_t�ṹ���phases[NGX_HTTP_CONTENT_PHASE]��̬���������ngx_http_handler_pt����������֮�����ϣ�������ʽ
��Ӧ����URIƥ�䶡ĳЩlocation���û�������Ӧ����һ��location��������Ļص������У���ngx_http_handler_pt�������õ�ngx_http_core_loc_conf_t
�ṹ���handler�С�
    ע��ngx_http_core_loc_conf_t�ṹ���н���һ��handlerָ�룬���������飬��Ҳ����ζ��������������ĵڶ��ַ������ngx_http_handler_pt��������
��ôÿ��������NGX_HTTP_CONTENT PHASE�׶�ֻ����һ��ngx_http_handler_pt����������ʹ�õ�һ�ַ���ʱ��û��������Ƶģ�NGX_HTTP_CONTENT_PHASE��
�ο��Ծ��������HTTPģ�鴦��
     */ //��handlerִ���ڣ����ȸ�ֵ��ngx_http_request_t->content_handler��Ȼ����ngx_http_core_content_phase��ִ�У���ֵ��ngx_http_update_location_config
    ngx_http_handler_pt  handler;/*HTTP����ڴ����û�������е�NGX_HTTP_CONTENT_PHASE�׶�ʱ��������������������URI��mytest���������ڵ����ÿ���ƥ�䣬�ͽ���������ʵ�ֵ�ngx_http_mytest_handler���������������*/

    /* location name length for inclusive location with inherited alias */
    size_t        alias;
    ngx_str_t     root;                    /* root, alias */
    ngx_str_t     post_action;

    ngx_array_t  *root_lengths;
    ngx_array_t  *root_values;

    ngx_array_t  *types;
    ngx_hash_t    types_hash;
    ngx_str_t     default_type;

    off_t         client_max_body_size;    /* client_max_body_size */
    off_t         directio;                /* directio */
    off_t         directio_alignment;      /* directio_alignment */
    //ʵ������client_body_buffer_size + client_body_buffer_size >> 2
    size_t        client_body_buffer_size; /* client_body_buffer_size */ 
    
    size_t        send_lowat;              /* send_lowat */ //���ø�ѡ��󣬻�����ngx_send_lowat
   /* 
   clcf->postpone_output�����ڴ���postpone_outputָ�����������ʱ�������ֵ������ָ�postpone s������������ݵ�sizeС��s��
   ���Ҳ������һ��buffer��Ҳ����Ҫflush����ô����ʱ�������ngx_http_write_filter
    */
    size_t        postpone_output;         /* postpone_output */
    size_t        limit_rate;              /* limit_rate */
    //�ڡ�������������Ϊ,����ʵ�ʵ�����Ӧ�ñ�limit_rate��΢��һ�㣬���Բο�ngx_http_write_filter
    size_t        limit_rate_after;        /* limit_rate_after */ 

    /*
     Syntax:  sendfile_max_chunk size;
     Default:  sendfile_max_chunk 0; 
     Context:  http, server, location
      
     When set to a non-zero value, limits the amount of data that can be transferred in a single sendfile() call. Without the 
     limit, one fast connection may seize the worker process entirely. ��������øò��������ܻ�����http��ܣ���Ϊ���ܷ��͵İ���ܴ�
     */ //���û�����ø�ֵ�����͵�ʱ��Ĭ��һ����෢��NGX_MAX_SIZE_T_VALUE - ngx_pagesize;  ��ngx_linux_sendfile_chain
    size_t        sendfile_max_chunk;      /* sendfile_max_chunk */ //���һ�η��͸��ͻ��˵����ݴ�С
    size_t        read_ahead;              /* read_ahead */
    
    //������ݰ�����ܴ󣬶Է����ܻ��η��Ͳ��ܷ�����ɣ�������Ҫ��ζ�ȡ���ȴ���ȡ�ͻ������ݵ��������ʱ�¼�Ϊ�ñ�������ngx_http_do_read_client_request_body
    ngx_msec_t    client_body_timeout;     /* client_body_timeout */ 
    ngx_msec_t    send_timeout;            /* send_timeout */
    ngx_msec_t    keepalive_timeout;       /* keepalive_timeout */ //�����յ��ͻ������󣬲�Ӧ���˺���ngx_http_set_keepalive���ñ��ʱ����Ĭ��75��

    //min(lingering_time,lingering_timeout)���ʱ���ڿ��Լ�����ȡ���ݣ�����ͻ����з������ݹ�������ngx_http_set_lingering_close
    ngx_msec_t    lingering_time;          /* lingering_time */
    ngx_msec_t    lingering_timeout;       /* lingering_timeout */ 

    
    ngx_msec_t    resolver_timeout;        /* resolver_timeout */

    ngx_resolver_t  *resolver;             /* resolver */

    time_t        keepalive_header;        /* keepalive_timeout */

    ngx_uint_t    keepalive_requests;      /* keepalive_requests */
    ngx_uint_t    keepalive_disable;       /* keepalive_disable */
    ngx_uint_t    satisfy;                 /* satisfy */ //ȡֵNGX_HTTP_SATISFY_ALL����NGX_HTTP_SATISFY_ANY  ��ngx_http_core_access_phase
/*
lingering_close
�﷨��lingering_close off | on | always;
Ĭ�ϣ�lingering_close on;
���ÿ飺http��server��location
�����ÿ���Nginx�ر��û����ӵķ�ʽ��always��ʾ�ر��û�����ǰ�����������ش��������������û����͵����ݡ�off��ʾ�ر�����ʱ��ȫ��������
���Ƿ��Ѿ���׼�������������û������ݡ�on���м�ֵ��һ��������ڹر�����ǰ���ᴦ�������ϵ��û����͵����ݣ�������Щ�������ҵ�����϶���֮��������ǲ���Ҫ�ġ�
*/
    ngx_uint_t    lingering_close;         /* lingering_close */
    ngx_uint_t    if_modified_since;       /* if_modified_since */
    ngx_uint_t    max_ranges;              /* max_ranges */
    ngx_uint_t    client_body_in_file_only; /* client_body_in_file_only */ //ȡֵNGX_HTTP_REQUEST_BODY_FILE_CLEAN��

    ngx_flag_t    client_body_in_single_buffer; //client_body_in_single_buffer on | off;����
                                           /* client_body_in_singe_buffer */
    ngx_flag_t    internal;                /* internal */
    ngx_flag_t    sendfile;                /* sendfile */ //sendfile on | off
    ngx_flag_t    aio;                     /* aio */
    ngx_flag_t    tcp_nopush;              /* tcp_nopush */
    ngx_flag_t    tcp_nodelay;             /* tcp_nodelay */
    ngx_flag_t    reset_timedout_connection; /* reset_timedout_connection */
    ngx_flag_t    server_name_in_redirect; /* server_name_in_redirect */
    ngx_flag_t    port_in_redirect;        /* port_in_redirect */
    ngx_flag_t    msie_padding;            /* msie_padding */
    ngx_flag_t    msie_refresh;            /* msie_refresh */
    ngx_flag_t    log_not_found;           /* log_not_found */
    ngx_flag_t    log_subrequest;          /* log_subrequest */
    ngx_flag_t    recursive_error_pages;   /* recursive_error_pages */
    ngx_flag_t    server_tokens;           /* server_tokens */
    ngx_flag_t    chunked_transfer_encoding; /* chunked_transfer_encoding */
    ngx_flag_t    etag;                    /* etag */

#if (NGX_HTTP_GZIP)
    ngx_flag_t    gzip_vary;               /* gzip_vary */

    ngx_uint_t    gzip_http_version;       /* gzip_http_version */
    ngx_uint_t    gzip_proxied;            /* gzip_proxied */

#if (NGX_PCRE)
    ngx_array_t  *gzip_disable;            /* gzip_disable */
#endif
#endif

#if (NGX_THREADS)
    ngx_thread_pool_t         *thread_pool;
    ngx_http_complex_value_t  *thread_pool_value;
#endif

#if (NGX_HAVE_OPENAT)
    ngx_uint_t    disable_symlinks;        /* disable_symlinks */
    ngx_http_complex_value_t  *disable_symlinks_from;
#endif

    ngx_array_t  *error_pages;             /* error_page */
    ngx_http_try_file_t    *try_files;     /* try_files */

    ngx_path_t   *client_body_temp_path;   /* client_body_temp_path */ //"client_body_temp_path"����

    ngx_open_file_cache_t  *open_file_cache;
    time_t        open_file_cache_valid;
    ngx_uint_t    open_file_cache_min_uses;
    ngx_flag_t    open_file_cache_errors;
    ngx_flag_t    open_file_cache_events;

    ngx_log_t    *error_log;

    ngx_uint_t    types_hash_max_size;
    ngx_uint_t    types_hash_bucket_size;

/*
ÿһ��server����Զ�Ӧ�Ŷ��location�飬��һ��location�黹���Լ���Ƕ�׶��location�顣ÿһ��location����ͨ��˫�����������ĸ����ÿ飨Ҫ
ô����server�飬Ҫô����location��{}����������
*/
    //ͷ����ngx_queue_t��next��ʼ�ĳ�ԱΪngx_http_location_queue_t
    //location{}�е����ô洢�����ڸ���server{}�����ĵ�ctx->loc_conf[ngx_http_core_module.ctx_index]->locations��
    //���е�location{}���ջ���ngx_http_init_locations������������Ȼ����ngx_http_init_static_location_trees������������
    ngx_queue_t  *locations;//ngx_http_add_location�����з���ռ�  location�е�loc����ͨ��������ӵ�������server{}��������loc_conf�У���ngx_http_add_location

#if 0
    ngx_http_core_loc_conf_t  *prev_location;
#endif
};

//��ʼ����ֵ��ngx_http_add_location  
//��������http���󣬼�ngx_http_init_locations
typedef struct {
    ngx_queue_t                      queue;//���е�loc����ͨ���ö���������һ��

    //��ȫƥ�� ������(@name)location  noname  ������ʽƥ��,�ڵ㶼��ӵ�exactָ��
    ngx_http_core_loc_conf_t        *exact; //������ʽ noname name ��ǰƥ���location���Ǵ浽�����棬��ngx_http_add_location
    ngx_http_core_loc_conf_t        *inclusive; //location ^~   Ӧ����ǰ׺ƥ��  ǰ׺ƥ��ngx_http_core_loc_conf_t�ڵ���ӵ���ָ����
    
    ngx_str_t                       *name;//��ǰlocation /xxx {}�е�/XXXX
    u_char                          *file_name; //���ڵ������ļ���
    ngx_uint_t                       line; //�������ļ��е��к�
    ngx_queue_t                      list; //�����������õ�����Ҫ��ǰ׺һ�����ַ���ͨ��list��ӣ�//�ܺõ�ͼ�⣬�ο�http://blog.csdn.net/fengmo_q/article/details/6683377
} ngx_http_location_queue_t;

//ngx_http_init_locations��name noname regex�����location(exact/inclusive ��ȫƥ��/ǰ׺ƥ��)
//ͼ��ο�:http://blog.csdn.net/fengmo_q/article/details/6683377
struct ngx_http_location_tree_node_s {
    ngx_http_location_tree_node_t   *left; 
    ngx_http_location_tree_node_t   *right;
    ngx_http_location_tree_node_t   *tree; //��ͨlocation��ĳ�ڵ��list��Ա�γɵ���  �޷���ȫƥ���location��ɵ���

    /*
    ���location��Ӧ��URIƥ���ַ��������ܹ���ȫƥ������ͣ���exactָ�����Ӧ��ngx_http_core_loc_conf_t�ṹ�壬����ΪNULL��ָ��
     */
    ngx_http_core_loc_conf_t        *exact;  //��ȷƥ��   ָ���γ��������Ķ���ngx_http_location_queue_t->exact
    ngx_http_core_loc_conf_t        *inclusive; //ǰ׺ƥ��

    u_char                           auto_redirect; //�Զ��ض����־
    u_char                           len;  //name�ַ�����ʵ�ʳ���
    u_char                           name[1]; //nameָ��location��Ӧ��URIƥ����ʽ
};


void ngx_http_core_run_phases(ngx_http_request_t *r);
ngx_int_t ngx_http_core_generic_phase(ngx_http_request_t *r,
    ngx_http_phase_handler_t *ph);
ngx_int_t ngx_http_core_rewrite_phase(ngx_http_request_t *r,
    ngx_http_phase_handler_t *ph);
ngx_int_t ngx_http_core_find_config_phase(ngx_http_request_t *r,
    ngx_http_phase_handler_t *ph);
ngx_int_t ngx_http_core_post_rewrite_phase(ngx_http_request_t *r,
    ngx_http_phase_handler_t *ph);
ngx_int_t ngx_http_core_access_phase(ngx_http_request_t *r,
    ngx_http_phase_handler_t *ph);
ngx_int_t ngx_http_core_post_access_phase(ngx_http_request_t *r,
    ngx_http_phase_handler_t *ph);
ngx_int_t ngx_http_core_try_files_phase(ngx_http_request_t *r,
    ngx_http_phase_handler_t *ph);
ngx_int_t ngx_http_core_content_phase(ngx_http_request_t *r,
    ngx_http_phase_handler_t *ph);


void *ngx_http_test_content_type(ngx_http_request_t *r, ngx_hash_t *types_hash);
ngx_int_t ngx_http_set_content_type(ngx_http_request_t *r);
void ngx_http_set_exten(ngx_http_request_t *r);
ngx_int_t ngx_http_set_etag(ngx_http_request_t *r);
void ngx_http_weak_etag(ngx_http_request_t *r);
ngx_int_t ngx_http_send_response(ngx_http_request_t *r, ngx_uint_t status,
    ngx_str_t *ct, ngx_http_complex_value_t *cv);
u_char *ngx_http_map_uri_to_path(ngx_http_request_t *r, ngx_str_t *name,
    size_t *root_length, size_t reserved);
ngx_int_t ngx_http_auth_basic_user(ngx_http_request_t *r);
#if (NGX_HTTP_GZIP)
ngx_int_t ngx_http_gzip_ok(ngx_http_request_t *r);
#endif


ngx_int_t ngx_http_subrequest(ngx_http_request_t *r,
    ngx_str_t *uri, ngx_str_t *args, ngx_http_request_t **sr,
    ngx_http_post_subrequest_t *psr, ngx_uint_t flags);
ngx_int_t ngx_http_internal_redirect(ngx_http_request_t *r,
    ngx_str_t *uri, ngx_str_t *args);
ngx_int_t ngx_http_named_location(ngx_http_request_t *r, ngx_str_t *name);


ngx_http_cleanup_t *ngx_http_cleanup_add(ngx_http_request_t *r, size_t size);

/*
����ģ��ĵ���˳��
    ��Ȼһ������ᱻ���е�HTTP����ģ�����δ�����ô��������һ����ЩHTTP����ģ���������֯��һ��ģ��Լ����ǵĵ���˳�������ȷ���ġ�
6.2.1  ������������ι��ɵ�
    �ڱ���NginxԴ����ʱ���Ѿ�������һ��������HTTP����ģ����ɵĵ����������������һ��������ǲ�һ���ģ���������ķ�������ÿһ��
Ԫ�ض���һ��������CԴ�����ļ��������CԴ�����ļ���ͨ������static��ָ̬�루�ֱ����ڴ���HTTPͷ����HTTP���壩��ָ����һ���ļ��еĹ��˷�����
��HTTP����ж���������ָ�룬ָ����������ĵ�һ��Ԫ�أ�Ҳ���ǵ�һ������HTTPͷ����HTTP����ķ�����
*/

/*
 ע�����HTTP����ģ����˵����ngx_modules�����е�λ��Խ������ʵ��ִ����
��ʱ��Խ����ִ�С���Ϊ�ڳ�ʼ��HTTP����ģ��ʱ��ÿһ��http����ģ�鶼�ǽ��Լ�����
��������������ײ��ġ�
*/
//ÿ������ģ�鴦��HTTPͷ���ķ�������������1������r��Ҳ���ǵ�ǰ������
typedef ngx_int_t (*ngx_http_output_header_filter_pt)(ngx_http_request_t *r);  //��ngx_http_top_header_filter
//ÿ������ģ�鴦��HTTP����ķ���ԭ�ͣ���������������-r��chain������r�ǵ�ǰ������chain��Ҫ���͵�HTTP����
typedef ngx_int_t (*ngx_http_output_body_filter_pt)(ngx_http_request_t *r, ngx_chain_t *chain);//��ngx_http_top_body_filter
typedef ngx_int_t (*ngx_http_request_body_filter_pt)
    (ngx_http_request_t *r, ngx_chain_t *chain);


ngx_int_t ngx_http_output_filter(ngx_http_request_t *r, ngx_chain_t *chain);
ngx_int_t ngx_http_write_filter(ngx_http_request_t *r, ngx_chain_t *chain);
ngx_int_t ngx_http_request_body_save_filter(ngx_http_request_t *r,
   ngx_chain_t *chain);


ngx_int_t ngx_http_set_disable_symlinks(ngx_http_request_t *r,
    ngx_http_core_loc_conf_t *clcf, ngx_str_t *path, ngx_open_file_info_t *of);

ngx_int_t ngx_http_get_forwarded_addr(ngx_http_request_t *r, ngx_addr_t *addr,
    ngx_array_t *headers, ngx_str_t *value, ngx_array_t *proxies,
    int recursive);


extern ngx_module_t  ngx_http_core_module;

extern ngx_uint_t ngx_http_max_module;

extern ngx_str_t  ngx_http_core_get_method;


#define ngx_http_clear_content_length(r)                                      \
                                                                              \
    r->headers_out.content_length_n = -1;                                     \
    if (r->headers_out.content_length) {                                      \
        r->headers_out.content_length->hash = 0;                              \
        r->headers_out.content_length = NULL;                                 \
    }

#define ngx_http_clear_accept_ranges(r)                                       \
                                                                              \
    r->allow_ranges = 0;                                                      \
    if (r->headers_out.accept_ranges) {                                       \
        r->headers_out.accept_ranges->hash = 0;                               \
        r->headers_out.accept_ranges = NULL;                                  \
    }

#define ngx_http_clear_last_modified(r)                                       \
                                                                              \
    r->headers_out.last_modified_time = -1;                                   \
    if (r->headers_out.last_modified) {                                       \
        r->headers_out.last_modified->hash = 0;                               \
        r->headers_out.last_modified = NULL;                                  \
    }

#define ngx_http_clear_location(r)                                            \
                                                                              \
    if (r->headers_out.location) {                                            \
        r->headers_out.location->hash = 0;                                    \
        r->headers_out.location = NULL;                                       \
    }

#define ngx_http_clear_etag(r)                                                \
                                                                              \
    if (r->headers_out.etag) {                                                \
        r->headers_out.etag->hash = 0;                                        \
        r->headers_out.etag = NULL;                                           \
    }


#endif /* _NGX_HTTP_CORE_H_INCLUDED_ */
