
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {//�����ж���Ч��ngx_output_chain
    ngx_bufs_t  bufs;  //output_buffers  5   3K  Ĭ��ֵoutput_buffers 1 32768
} ngx_http_copy_filter_conf_t;


#if (NGX_HAVE_FILE_AIO)
static void ngx_http_copy_aio_handler(ngx_output_chain_ctx_t *ctx,
    ngx_file_t *file);
static void ngx_http_copy_aio_event_handler(ngx_event_t *ev);
#if (NGX_HAVE_AIO_SENDFILE)
static ssize_t ngx_http_copy_aio_sendfile_preload(ngx_buf_t *file);
static void ngx_http_copy_aio_sendfile_event_handler(ngx_event_t *ev);
#endif
#endif
#if (NGX_THREADS)
static ngx_int_t ngx_http_copy_thread_handler(ngx_thread_task_t *task,
    ngx_file_t *file);
static void ngx_http_copy_thread_event_handler(ngx_event_t *ev);
#endif

static void *ngx_http_copy_filter_create_conf(ngx_conf_t *cf);
static char *ngx_http_copy_filter_merge_conf(ngx_conf_t *cf,
    void *parent, void *child);
static ngx_int_t ngx_http_copy_filter_init(ngx_conf_t *cf);


static ngx_command_t  ngx_http_copy_filter_commands[] = {

    { ngx_string("output_buffers"), //�����ж���Ч��ngx_output_chain
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE2,
      ngx_conf_set_bufs_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_copy_filter_conf_t, bufs),
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_copy_filter_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_copy_filter_init,             /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_copy_filter_create_conf,      /* create location configuration */
    ngx_http_copy_filter_merge_conf        /* merge location configuration */
};

/*
��6-1  Ĭ�ϼ������Nginx��HTTP����ģ��
���������������������������������������ש�������������������������������������������������������������������
��Ĭ�ϼ������Nginx��HTTP����ģ��     ��    ����                                                          ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ����HTTPͷ���������ڷ���200�ɹ�ʱ������������If-              ��
��                                    ��Modified-Since����If-Unmodified-Sinceͷ��ȡ������������ļ���ʱ   ��
��ngx_http_not_modified_filter_module ��                                                                  ��
��                                    ���䣬�ٷ��������û��ļ�������޸�ʱ�䣬�Դ˾����Ƿ�ֱ�ӷ���304     ��
��                                    �� Not Modified��Ӧ���û�                                           ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ���������е�Range��Ϣ������Range�е�Ҫ�󷵻��ļ���һ���ָ�      ��
��ngx_http_range_body_filter_module   ��                                                                  ��
��                                    ���û�                                                              ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ����HTTP�������������û����͵�ngx_chain_t�ṹ��HTTP��         ��
��                                    ���帴�Ƶ��µ�ngx_chain_t�ṹ�У����Ǹ���ָ��ĸ��ƣ�������ʵ��     ��
��ngx_http_copy_filter_module         ��                                                                  ��
��                                    ��HTTP��Ӧ���ݣ���������HTTP����ģ�鴦���ngx_chain_t���͵ĳ�       ��
��                                    ��Ա����ngx_http_copy_filter_moduleģ�鴦���ı���                 ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ����HTTPͷ������������ͨ���޸�nginx.conf�����ļ����ڷ���      ��
��ngx_http_headers_filter_module      ��                                                                  ��
��                                    �����û�����Ӧ����������HTTPͷ��                                  ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ����HTTPͷ�������������ִ��configure����ʱ�ᵽ��http_        ��
��ngx_http_userid_filter_module       ��                                                                  ��
��                                    ��userid moduleģ�飬������cookie�ṩ�˼򵥵���֤������           ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ���Խ��ı����ͷ��ظ��û�����Ӧ��������nginx��conf�е���������   ��
��ngx_http_charset_filter_module      ��                                                                  ��
��                                    �����б��룬�ٷ��ظ��û�                                            ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ֧��SSI��Server Side Include����������Ƕ�룩���ܣ����ļ����ݰ�  ��
��ngx_http_ssi_filter_module          ��                                                                  ��
��                                    ��������ҳ�в����ظ��û�                                            ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ����HTTP����������5.5.2����ϸ���ܹ��ù���ģ�顣����Ӧ����     ��
��ngx_http_postpone_filter_module     ��subrequest��������������ʹ�ö��������ͬʱ��ͻ��˷�����Ӧʱ    ��
��                                    ���ܹ�������ν�ġ������ǿ����չ����������˳������Ӧ            ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ���ض���HTTP��Ӧ���壨����ҳ�����ı��ļ�������gzipѹ������      ��
��ngx_http_gzip_filter_module         ��                                                                  ��
��                                    ����ѹ��������ݷ��ظ��û�                                          ��
�ǩ������������������������������������贈������������������������������������������������������������������
��ngx_http_range_header_filter_module ��  ֧��rangeЭ��                                                   ��
�ǩ������������������������������������贈������������������������������������������������������������������
��ngx_http_chunked_filter_module      ��  ֧��chunk����                                                   ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ����HTTPͷ���������ù���ģ�齫���r->headers out�ṹ��        ��
��                                    ���еĳ�Ա���л�Ϊ���ظ��û���HTTP��Ӧ�ַ�����������Ӧ��(��         ��
��ngx_http_header_filter_module       ��                                                                  ��
��                                    ��HTTP/I.1 200 0K)����Ӧͷ������ͨ������ngx_http_write filter       ��
��                                    �� module����ģ���еĹ��˷���ֱ�ӽ�HTTP��ͷ���͵��ͻ���             ��
�ǩ������������������������������������贈������������������������������������������������������������������
��ngx_http_write_filter_module        ��  ����HTTP������������ģ�鸺����ͻ��˷���HTTP��Ӧ              ��
���������������������������������������ߩ�������������������������������������������������������������������

*/

ngx_module_t  ngx_http_copy_filter_module = {
    NGX_MODULE_V1,
    &ngx_http_copy_filter_module_ctx,      /* module context */
    ngx_http_copy_filter_commands,         /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_http_output_body_filter_pt    ngx_http_next_body_filter;


/*
2015/12/31 06:36:20[ ngx_event_pipe_write_to_downstream,   600]  [debug] 21359#21359: *1 pipe write downstream, write ready: 1
2015/12/31 06:36:20[ ngx_event_pipe_write_to_downstream,   620]  [debug] 21359#21359: *1 pipe write downstream flush out
2015/12/31 06:36:20[             ngx_http_output_filter,  3338]  [debug] 21359#21359: *1 http output filter "/test2.php?"
2015/12/31 06:36:20[               ngx_http_copy_filter,   160]  [debug] 21359#21359: *1 http copy filter: "/test2.php?", r->aio:0
2015/12/31 06:36:20[                   ngx_output_chain,    67][yangya  [debug] 21359#21359: *1 ctx->sendfile:0, ctx->aio:0, ctx->directio:0
2015/12/31 06:36:20[                  ngx_file_aio_read,   251]  [debug] 21359#21359: *1 aio complete:0 @206:215 /var/yyz/cache_xxx/temp/1/00/0000000001
2015/12/31 06:36:20[               ngx_http_copy_filter,   231]  [debug] 21359#21359: *1 http copy filter: -2 "/test2.php?"
2015/12/31 06:36:20[ ngx_event_pipe_write_to_downstream,   658]  [debug] 21359#21359: *1 pipe write downstream done
2015/12/31 06:36:20[                ngx_event_del_timer,    39]  [debug] 21359#21359: *1 <           ngx_event_pipe,    87>  event timer del: 14: 4111061275
2015/12/31 06:36:20[         ngx_http_file_cache_update,  1508]  [debug] 21359#21359: *1 http file cache update, c->body_start:206
2015/12/31 06:36:20[         ngx_http_file_cache_update,  1521]  [debug] 21359#21359: *1 http file cache rename: "/var/yyz/cache_xxx/temp/1/00/0000000001" to "/var/yyz/cache_xxx/f/27/46492fbf0d9d35d3753c66851e81627f", expire time:1800
2015/12/31 06:36:20[                     ngx_shmtx_lock,   168]  [debug] 21359#21359: shmtx lock
2015/12/31 06:36:20[                   ngx_shmtx_unlock,   249]  [debug] 21359#21359: shmtx unlock
2015/12/31 06:36:20[  ngx_http_upstream_process_request,  4250]  [debug] 21359#21359: *1 http upstream exit: 00000000
2015/12/31 06:36:20[ ngx_http_upstream_finalize_request,  4521]  [debug] 21359#21359: *1 finalize http upstream request rc: 0
2015/12/31 06:36:20[  ngx_http_fastcgi_finalize_request,  3215]  [debug] 21359#21359: *1 finalize http fastcgi request
2015/12/31 06:36:20[ngx_http_upstream_free_round_robin_peer,   887]  [debug] 21359#21359: *1 free rr peer 1 0
2015/12/31 06:36:20[ ngx_http_upstream_finalize_request,  4574]  [debug] 21359#21359: *1 close http upstream connection: 14
2015/12/31 06:36:20[               ngx_close_connection,  1120]  [debug] 21359#21359: *1 delete posted event AE8FD098
2015/12/31 06:36:20[            ngx_reusable_connection,  1177]  [debug] 21359#21359: *1 reusable connection: 0
2015/12/31 06:36:20[               ngx_close_connection,  1139][yangya  [debug] 21359#21359: close socket:14
2015/12/31 06:36:20[ ngx_http_upstream_finalize_request,  4588]  [debug] 21359#21359: *1 http upstream temp fd: 15
2015/12/31 06:36:20[              ngx_http_send_special,  3843][yangya  [debug] 21359#21359: *1 ngx http send special, flags:1
2015/12/31 06:36:20[             ngx_http_output_filter,  3338]  [debug] 21359#21359: *1 http output filter "/test2.php?"
2015/12/31 06:36:20[               ngx_http_copy_filter,   160]  [debug] 21359#21359: *1 http copy filter: "/test2.php?", r->aio:1
2015/12/31 06:36:20[                   ngx_output_chain,    67][yangya  [debug] 21359#21359: *1 ctx->sendfile:0, ctx->aio:1, ctx->directio:0
2015/12/31 06:36:20[               ngx_http_copy_filter,   231]  [debug] 21359#21359: *1 http copy filter: -2 "/test2.php?"
2015/12/31 06:36:20[          ngx_http_finalize_request,  2592]  [debug] 21359#21359: *1 http finalize request rc: -2, "/test2.php?" a:1, c:1
2015/12/31 06:36:20[                ngx_event_add_timer,   100]  [debug] 21359#21359: *1 <ngx_http_set_write_handler,  3013>  event timer add fd:13, expire-time:60 s, timer.key:4111061277
2015/12/31 06:36:20[           ngx_trylock_accept_mutex,   405]  [debug] 21359#21359: accept mutex locked
2015/12/31 06:36:20[           ngx_epoll_process_events,  1713]  [debug] 21359#21359: epoll: fd:10 EPOLLIN  (ev:0001) d:080E2520
2015/12/31 06:36:20[           ngx_epoll_process_events,  1759]  [debug] 21359#21359: post event 080E24E0
2015/12/31 06:36:20[           ngx_event_process_posted,    65]  [debug] 21359#21359: begin to run befor posted event 080E24E0

*/

/*
ͨ�����﷢�ʹ�����ngx_http_write_filter->ngx_linux_sendfile_chain(����ļ�ͨ��sendfile����)��
�������ͨд���ͣ�����ngx_http_write_filter->ngx_writev(һ��chain->buf���ڴ��е�������ø÷�ʽ)��
����ngx_http_copy_filter->ngx_output_chain�е�if (ctx->aio) { return NGX_AGAIN;}(����ļ�ͨ��aio����)��Ȼ����aio�첽�¼�epoll����
��ȡ�ļ����ݳ�����Ȼ���ڼ��������ļ�
*/

/* ע��:�������inʵ�������Ѿ�ָ���������ݲ��֣�����������͵�������Ҫ���ļ��ж�ȡ��in��Ҳ��ָ���ļ�file_pos��file_last�Ѿ��ļ�fd��,
   ���Բο�ngx_http_cache_send ngx_http_send_header ngx_http_output_filter */
//inΪ��Ҫ���͵�chain��������洢����ʵ��Ҫ���͵�����
static ngx_int_t
ngx_http_copy_filter(ngx_http_request_t *r, ngx_chain_t *in)
{ //ʵ�����ڽ����������ݺ�����ͻ��˷��Ͱ��岿�ֵ�ʱ�򣬻����ε��øú�����һ����ngx_event_pipe_write_to_downstream-> p->output_filter(),
//��һ����ngx_http_upstream_finalize_request->ngx_http_send_special,���Բο��������־��ӡע��
    ngx_int_t                     rc;
    ngx_connection_t             *c;
    ngx_output_chain_ctx_t       *ctx;
    ngx_http_core_loc_conf_t     *clcf;
    ngx_http_copy_filter_conf_t  *conf;
    int aio = r->aio;

    c = r->connection;

    ngx_log_debug3(NGX_LOG_DEBUG_HTTP, c->log, 0,
                   "http copy filter: \"%V?%V\", r->aio:%d", &r->uri, &r->args, aio);

    ctx = ngx_http_get_module_ctx(r, ngx_http_copy_filter_module);

    if (ctx == NULL) {
        ctx = ngx_pcalloc(r->pool, sizeof(ngx_output_chain_ctx_t));
        if (ctx == NULL) {
            return NGX_ERROR;
        }

        ngx_http_set_ctx(r, ctx, ngx_http_copy_filter_module);

        conf = ngx_http_get_module_loc_conf(r, ngx_http_copy_filter_module);
        clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);

    /*
        �ͺ�˵�ngx_connection_t��ngx_event_connect_peer������Ϊ1������ngx_http_upstream_connect��c->sendfile &= r->connection->sendfile;��
        �Ϳͻ����������ngx_connextion_t��sendfile��Ҫ��ngx_http_update_location_config���жϣ�������������Ƿ���configure��ʱ���Ƿ��м�
        sendfileѡ������������1������0
     */
        ctx->sendfile = c->sendfile;
        ctx->need_in_memory = r->main_filter_need_in_memory
                              || r->filter_need_in_memory;
        ctx->need_in_temp = r->filter_need_temporary;

        ctx->alignment = clcf->directio_alignment;

        ctx->pool = r->pool;
        ctx->bufs = conf->bufs; // Ĭ��ֵoutput_buffers 1 32768
        ctx->tag = (ngx_buf_tag_t) &ngx_http_copy_filter_module;

        ctx->output_filter = (ngx_output_chain_filter_pt)
                                  ngx_http_next_body_filter;
        ctx->filter_ctx = r;

#if (NGX_HAVE_FILE_AIO)
        if (ngx_file_aio && clcf->aio == NGX_HTTP_AIO_ON) { //./configure��ʱ�����--with-file-aio���������ļ���aio on��ʱ�����Ч
            ctx->aio_handler = ngx_http_copy_aio_handler;
#if (NGX_HAVE_AIO_SENDFILE) //ֻ��freebsdϵͳ����Ч auto/os/freebsd:        have=NGX_HAVE_AIO_SENDFILE . auto/have
            ctx->aio_preload = ngx_http_copy_aio_sendfile_preload;
#endif
        }
#endif

#if (NGX_THREADS) 
        if (clcf->aio == NGX_HTTP_AIO_THREADS) {
            //ngx_output_chain_as_is�и�ֵ��buf->file->thread_handler 
            ctx->thread_handler = ngx_http_copy_thread_handler;
        }
#endif

        //һ���ڵ���filter������Դͷ������in��ָ����Ҫ���͵����ݳ��ȣ����Բο�ngx_http_cache_send
        if (in && in->buf && ngx_buf_size(in->buf)) { //�ж�in�����Ƿ�������
            r->request_output = 1;
        }
    }

#if (NGX_HAVE_FILE_AIO || NGX_THREADS)
    //ʵ�����ڽ����������ݺ�����ͻ��˷��Ͱ��岿�ֵ�ʱ�򣬻����ε��øú�����һ����ngx_event_pipe_write_to_downstream-> p->output_filter(),
    //��һ����ngx_http_upstream_finalize_request->ngx_http_send_special,
    //�����aio��ʽ�����һ�θ�ֵΪ0�����ǵڶ��δ�ngx_http_send_special�ߵ������ʱ���Ѿ���ngx_output_chain->ngx_file_aio_read->ngx_http_copy_aio_handler��1
    //aio��ʽ����aio���¼���ɣ���ͨ��ngx_http_copy_aio_event_handler->ngx_http_writer�ٴ��ߵ������ʱ��ngx_http_copy_aio_event_handler�Ѿ���r->aio��0
    //���Բο��������־��ע��Ϣ
    ctx->aio = r->aio; 
#endif

    rc = ngx_output_chain(ctx, in);

    if (ctx->in == NULL) {
        r->buffered &= ~NGX_HTTP_COPY_BUFFERED;

    } else {//˵����������δ���͵��ͻ���r
    //ngx_http_finalize_request->ngx_http_set_write_handler->ngx_http_writerͨ�����ַ�ʽ��δ������ϵ���Ӧ���ķ��ͳ�ȥ
        r->buffered |= NGX_HTTP_COPY_BUFFERED; //˵��ctx->in�ϻ���δ���͵����ݣ���������in��ָ����ngx_output_chain���Ѿ���ֵ����ctx->in
    }

    ngx_int_t buffered = r->buffered;
    ngx_log_debug4(NGX_LOG_DEBUG_HTTP, c->log, 0,
                   "http copy filter rc: %i, buffered:%i \"%V?%V\"", rc, buffered, &r->uri, &r->args);

    return rc;
}


#if (NGX_HAVE_FILE_AIO)
//ִ����ngx_output_chain_copy_buf->ngx_http_copy_aio_handler
static void
ngx_http_copy_aio_handler(ngx_output_chain_ctx_t *ctx, ngx_file_t *file)
{ //ע��aio�ں˶�ȡ��Ϻ��Ƿ���ngx_output_chain_ctx_t->buf�еģ���ngx_output_chain_copy_buf->ngx_file_aio_read
    ngx_http_request_t *r;

    r = ctx->filter_ctx;

    file->aio->data = r;
    file->aio->handler = ngx_http_copy_aio_event_handler;

    r->main->blocked++;
    r->aio = 1;
    ctx->aio = 1;
}

//ע��aio�ں˶�ȡ��Ϻ��Ƿ���ngx_output_chain_ctx_t->buf�еģ���ngx_output_chain_copy_buf->ngx_file_aio_read

//ngx_file_aio_event_handler��ִ��
static void
ngx_http_copy_aio_event_handler(ngx_event_t *ev)
{
    ngx_event_aio_t     *aio;
    ngx_http_request_t  *r;

    aio = ev->data;
    r = aio->data;

    r->main->blocked--;  
    r->aio = 0;

    //ngx_http_request_handler -> ngx_http_writer   ngx_http_set_write_handler������Ϊngx_http_writer
    r->connection->write->handler(r->connection->write); //����һ��write->handler���Ӷ����԰Ѵ�ngx_file_aio_read���������ݷ��ͳ�ȥ
}


#if (NGX_HAVE_AIO_SENDFILE)

static ssize_t
ngx_http_copy_aio_sendfile_preload(ngx_buf_t *file)
{
    ssize_t              n;
    static u_char        buf[1];
    ngx_event_aio_t     *aio;
    ngx_http_request_t  *r;

    n = ngx_file_aio_read(file->file, buf, 1, file->file_pos, NULL);

    if (n == NGX_AGAIN) {
        aio = file->file->aio;
        aio->handler = ngx_http_copy_aio_sendfile_event_handler;

        r = aio->data;
        r->main->blocked++;
        r->aio = 1;
    }

    return n;
}


static void
ngx_http_copy_aio_sendfile_event_handler(ngx_event_t *ev)
{
    ngx_event_aio_t     *aio;
    ngx_http_request_t  *r;

    aio = ev->data;
    r = aio->data;

    r->main->blocked--;
    r->aio = 0;
    ev->complete = 0;

    r->connection->write->handler(r->connection->write);
}

#endif
#endif


#if (NGX_THREADS)

static ngx_int_t
ngx_http_copy_thread_handler(ngx_thread_task_t *task, ngx_file_t *file)
{
    ngx_str_t                  name;
    ngx_thread_pool_t         *tp;
    ngx_http_request_t        *r;
    ngx_http_core_loc_conf_t  *clcf;

    r = file->thread_ctx;

    clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);
    tp = clcf->thread_pool;

    if (tp == NULL) {
        if (ngx_http_complex_value(r, clcf->thread_pool_value, &name)
            != NGX_OK)
        {
            return NGX_ERROR;
        }

        tp = ngx_thread_pool_get((ngx_cycle_t *) ngx_cycle, &name);

        if (tp == NULL) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                          "thread pool \"%V\" not found", &name);
            return NGX_ERROR;
        }
    }

    task->event.data = r;
    task->event.handler = ngx_http_copy_thread_event_handler;

    if (ngx_thread_task_post(tp, task) != NGX_OK) {
        return NGX_ERROR;
    }

    r->main->blocked++;
    r->aio = 1;

    return NGX_OK;
}


static void
ngx_http_copy_thread_event_handler(ngx_event_t *ev)
{
    ngx_http_request_t  *r;

    r = ev->data;

    r->main->blocked--;
    r->aio = 0;

    //ngx_http_request_handler -> ngx_http_writer
    r->connection->write->handler(r->connection->write);
}

#endif


static void *
ngx_http_copy_filter_create_conf(ngx_conf_t *cf)
{
    ngx_http_copy_filter_conf_t *conf;

    conf = ngx_palloc(cf->pool, sizeof(ngx_http_copy_filter_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->bufs.num = 0;

    return conf;
}


static char *
ngx_http_copy_filter_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_copy_filter_conf_t *prev = parent;
    ngx_http_copy_filter_conf_t *conf = child;

    ngx_conf_merge_bufs_value(conf->bufs, prev->bufs, 1, 32768);

    return NULL;
}


static ngx_int_t
ngx_http_copy_filter_init(ngx_conf_t *cf)
{
    ngx_http_next_body_filter = ngx_http_top_body_filter;
    ngx_http_top_body_filter = ngx_http_copy_filter;

    return NGX_OK;
}

