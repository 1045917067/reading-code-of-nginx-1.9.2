
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>

//ngx_linux_sendfile_chain��ngx_writev_chain
ngx_chain_t *
ngx_writev_chain(ngx_connection_t *c, ngx_chain_t *in, off_t limit)
{//����writevһ�η��Ͷ�������������û�з�����ϣ��򷵻�ʣ�µ����ӽṹͷ����
//ngx_chain_writer����������÷�ʽΪ ctx->out = c->send_chain(c, ctx->out, ctx->limit);
//�ڶ�������ΪҪ���͵�����
    ssize_t        n, sent;
    off_t          send, prev_send;
    ngx_chain_t   *cl;
    ngx_event_t   *wev;
    ngx_iovec_t    vec;
    struct iovec   iovs[NGX_IOVS_PREALLOCATE];

    wev = c->write;//�õ�������ӵ�д�¼��ṹ

    if (!wev->ready) {//���ӻ�û׼���ã����ص�ǰ�Ľڵ㡣
        return in;
    }

#if (NGX_HAVE_KQUEUE)

    if ((ngx_event_flags & NGX_USE_KQUEUE_EVENT) && wev->pending_eof) {
        (void) ngx_connection_error(c, wev->kq_errno,
                               "kevent() reported about an closed connection");
        wev->error = 1;
        return NGX_CHAIN_ERROR;
    }

#endif

    /* the maximum limit size is the maximum size_t value - the page size */

    if (limit == 0 || limit > (off_t) (NGX_MAX_SIZE_T_VALUE - ngx_pagesize)) {
        limit = NGX_MAX_SIZE_T_VALUE - ngx_pagesize;//�����ˣ���������
    }

    send = 0;

    vec.iovs = iovs;
    vec.nalloc = NGX_IOVS_PREALLOCATE;

    for ( ;; ) {
        prev_send = send; //prev_sendΪ��һ�ε���ngx_writev���ͳ�ȥ���ֽ���

        /* create the iovec and coalesce the neighbouring bufs */

        cl = ngx_output_chain_to_iovec(&vec, in, limit - send, c->log);

        if (cl == NGX_CHAIN_ERROR) {
            return NGX_CHAIN_ERROR;
        }

        if (cl && cl->buf->in_file) {
            ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                          "file buf in writev "
                          "t:%d r:%d f:%d %p %p-%p %p %O-%O",
                          cl->buf->temporary,
                          cl->buf->recycled,
                          cl->buf->in_file,
                          cl->buf->start,
                          cl->buf->pos,
                          cl->buf->last,
                          cl->buf->file,
                          cl->buf->file_pos,
                          cl->buf->file_last);

            ngx_debug_point();

            return NGX_CHAIN_ERROR;
        }

        send += vec.size; //Ϊngx_output_chain_to_iovec�������in�����������ݳ��Ⱥ�

        n = ngx_writev(c, &vec); 
        //����������vec->size�ֽ����ݣ�����ʵ�����ں˷��ͳ�ȥ�ĺܿ��ܱ�vec->sizeС��nΪʵ�ʷ��ͳ�ȥ���ֽ����������Ҫ��������

        if (n == NGX_ERROR) {
            return NGX_CHAIN_ERROR;
        }

        sent = (n == NGX_AGAIN) ? 0 : n;//��¼���͵����ݴ�С��

        c->sent += sent;//����ͳ�����ݣ���������Ϸ��͵����ݴ�С

        in = ngx_chain_update_sent(in, sent); //send�Ǵ˴ε���ngx_wrtev���ͳɹ����ֽ���
        //ngx_chain_update_sent���غ��in���Ѿ�������֮ǰ���ͳɹ���in�ڵ��ˣ�������ֻ����ʣ�������
        
        if (send - prev_send != sent) { //����˵��������ngx_writev���γɹ����ͺ�����ͻ᷵��
            wev->ready = 0; //�����ʱ���ܷ��������ˣ���������epoll_addд�¼�
            return in;
        }

        if (send >= limit || in == NULL) { //���ݷ�����ϣ����߱��η��ͳɹ����ֽ�����limit���࣬�򷵻س�ȥ
            return in; //
        }
    }
}

//��in���е�buf������vec->iovs[n++]��
ngx_chain_t *
ngx_output_chain_to_iovec(ngx_iovec_t *vec, ngx_chain_t *in, size_t limit,
    ngx_log_t *log)
{
    size_t         total, size;
    u_char        *prev;
    ngx_uint_t     n;
    struct iovec  *iov;

    iov = NULL;
    prev = NULL;
    total = 0;
    n = 0;
    //ѭ���������ݣ�һ��һ��IOV_MAX��Ŀ�Ļ�������
    for ( /* void */ ; in && total < limit; in = in->next) {

        if (ngx_buf_special(in->buf)) {
            continue;
        }

        if (in->buf->in_file) { //���Ϊ1,��ʾ��sendfile���ͣ���ngx_output_chain_copy_buf
            break;
        }

        if (!ngx_buf_in_memory(in->buf)) {
            ngx_log_error(NGX_LOG_ALERT, log, 0,
                          "bad buf in output chain "
                          "t:%d r:%d f:%d %p %p-%p %p %O-%O",
                          in->buf->temporary,
                          in->buf->recycled,
                          in->buf->in_file,
                          in->buf->start,
                          in->buf->pos,
                          in->buf->last,
                          in->buf->file,
                          in->buf->file_pos,
                          in->buf->file_last);

            ngx_debug_point();

            return NGX_CHAIN_ERROR;
        }

        size = in->buf->last - in->buf->pos;//��������ڵ�Ĵ�С

        if (size > limit - total) {//��������ʹ�С���ضϣ����ֻ������ô��
            size = limit - total;
        }

        if (prev == in->buf->pos) {//������ǵ��ڸղŵ�λ�ã��Ǿ͸���
            iov->iov_len += size;

        } else {//����Ҫ����һ���ڵ㡣����֮
            if (n == vec->nalloc) {
                break;
            }

            iov = &vec->iovs[n++];

            iov->iov_base = (void *) in->buf->pos;//�����￪ʼ
            iov->iov_len = size;//����ô����Ҫ����
        }

        prev = in->buf->pos + size;//��¼�ղŷ��������λ�ã�Ϊָ�롣
        total += size;//�����Ѿ���¼�����ݳ��ȡ�
    }

    vec->count = n;
    vec->size = total;

    return in;
}


ssize_t
ngx_writev(ngx_connection_t *c, ngx_iovec_t *vec)
{
    ssize_t    n;
    ngx_err_t  err;

eintr:
    //����writev������Щ���ݣ����ط��͵����ݴ�С
    //readv ��writev����һ�¶�д��������������ݣ�read��writeֻ��һ�¶�дһ�������������ݣ� 
    /* On success, the readv() function returns the number of bytes read; the writev() function returns the number of bytes written.  
        On error, -1 is returned, and errno is  set appropriately. readv���ر������ֽ����������û�и������ݺ������ļ�ĩβʱ����0�ļ����� */
    n = writev(c->fd, vec->iovs, vec->count);

    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, c->log, 0,
                   "writev: %z of %uz", n, vec->size);

    if (n == -1) {
        err = ngx_errno;

        switch (err) {
        case NGX_EAGAIN:
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, err,
                           "writev() not ready");
            return NGX_AGAIN;

        case NGX_EINTR:
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, err,
                           "writev() was interrupted");
            goto eintr;

        default:
            c->write->error = 1;
            ngx_connection_error(c, err, "writev() failed");
            return NGX_ERROR;
        }
    }

    return n;
}
