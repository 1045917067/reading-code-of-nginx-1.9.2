
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


#if (NGX_HAVE_KQUEUE)

ssize_t
ngx_unix_recv(ngx_connection_t *c, u_char *buf, size_t size)
{
    ssize_t       n;
    ngx_err_t     err;
    ngx_event_t  *rev;

    rev = c->read;

    if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "recv: eof:%d, avail:%d, err:%d",
                       rev->pending_eof, rev->available, rev->kq_errno);

        if (rev->available == 0) {
            if (rev->pending_eof) {
                rev->ready = 0;
                rev->eof = 1;

                if (rev->kq_errno) {
                    rev->error = 1;
                    ngx_set_socket_errno(rev->kq_errno);

                    return ngx_connection_error(c, rev->kq_errno,
                               "kevent() reported about an closed connection");
                }

                return 0;

            } else {
                rev->ready = 0;
                return NGX_AGAIN;
            }
        }
    }

    do {
        n = recv(c->fd, buf, size, 0);

        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "recv: fd:%d %d of %d", c->fd, n, size);

        if (n >= 0) {
            if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
                rev->available -= n;

                /*
                 * rev->available may be negative here because some additional
                 * bytes may be received between kevent() and recv()
                 */

                if (rev->available <= 0) {
                    if (!rev->pending_eof) {
                        rev->ready = 0;
                    }

                    if (rev->available < 0) {
                        rev->available = 0;
                    }
                }

                if (n == 0) {

                    /*
                     * on FreeBSD recv() may return 0 on closed socket
                     * even if kqueue reported about available data
                     */

                    rev->ready = 0;
                    rev->eof = 1;
                    rev->available = 0;
                }

                return n;
            }

            if ((size_t) n < size
                && !(ngx_event_flags & NGX_USE_GREEDY_EVENT))
            {
                rev->ready = 0;
            }

            if (n == 0) {
                rev->eof = 1;
            }

            return n;
        }

        err = ngx_socket_errno;

        if (err == NGX_EAGAIN || err == NGX_EINTR) {
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, err,
                           "recv() not ready");
            n = NGX_AGAIN;

        } else {
            n = ngx_connection_error(c, err, "recv() failed");
            break;
        }

    } while (err == NGX_EINTR);

    rev->ready = 0;

    if (n == NGX_ERROR) {
        rev->error = 1;
    }

    return n;
}

#else /* ! NGX_HAVE_KQUEUE */

ssize_t
ngx_unix_recv(ngx_connection_t *c, u_char *buf, size_t size)
{
    ssize_t       n;
    ngx_err_t     err;
    ngx_event_t  *rev;

    rev = c->read;

    do {
        /*
            ��Է�����I/Oִ�е�ϵͳ�����������������أ��������¼�����Ѿ�����������¼�û��������������Щϵͳ���þ�
        ���ء�1���ͳ�������һ������ʱ���Ǳ������errno�������������������accept��send��recv���ԣ��¼�δ��ţʱerrno
        ͨ�������ó�EAGAIN����Ϊ������һ�Ρ�������EWOULDBLOCK����Ϊ���ڴ�������������conncct���ԣ�errno��
        ���ó�EINPROGRESS����Ϊ���ڴ�����"����
          */
        //n = recv(c->fd, buf, size, 0); yang test
        n = recv(c->fd, buf, size, 0);//��ʾTCP���󣬼�ngx_http_read_request_header   recv����0��ʾ�Է��Ѿ��ر�����

        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, c->log, 0,
                           "recv: fd:%d read-size:%d of %d", c->fd, n, size);

        //��ȡ�ɹ���ֱ�ӷ���   
        if (n == 0) { //��ʾTCP���󣬼�ngx_http_read_request_header   recv����0��ʾ�Է��Ѿ��ر�����
            rev->ready = 0;//���ݶ�ȡ���ready��0
            rev->eof = 1;
            return n;

        } else if (n > 0) {

            if ((size_t) n < size
                && !(ngx_event_flags & NGX_USE_GREEDY_EVENT)) //���ݶ�ȡ���ready��0,��Ҫ�������add epoll event
            {
                rev->ready = 0;
            }

            return n;
        }

        //����ں����ݽ�����ϣ����ߵ�����nΪ-1��errΪNGX_EAGAIN
        err = ngx_socket_errno;
        
        /* 
          EINTR����Ĳ�������������ĳ����ϵͳ���õ�һ�����̲���ĳ���ź�����Ӧ�źŴ���������ʱ����ϵͳ���ÿ��ܷ���һ��EINTR����   
          ��linux���з�������socket��������ʱ��������Resource temporarily unavailable��errno����Ϊ11(EAGAIN)����������ڷ�����ģʽ�µ���������������
          �ڸò���û����ɾͷ����������������󲻻��ƻ�socket��ͬ�������ù������´�ѭ������recv�Ϳ��ԡ��Է�����socket���ԣ�EAGAIN����һ�ִ���
          ��VxWorks��Windows�ϣ�EAGAIN�����ֽ���EWOULDBLOCK�� ���⣬�������EINTR��errnoΪ4����������Interrupted system call������ҲӦ�ü�����
          */
        if (err == NGX_EAGAIN || err == NGX_EINTR) {  //��������� ,��Ҫ������
            
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, err,
                           "recv() not ready"); //recv() not ready (11: Resource temporarily unavailable)
            n = NGX_AGAIN; //���������ʾ�ں��������е������Ѿ���ȡ�꣬��Ҫ����add epoll event������������epoll����

        } else {//TCP���ӳ�����
            n = ngx_connection_error(c, err, "recv() failed");
            break;
        }

    } while (err == NGX_EINTR); //����������б��ж��л����������

    rev->ready = 0;

    if (n == NGX_ERROR) {
        rev->error = 1;
    }

    return n;
}

#endif /* NGX_HAVE_KQUEUE */
