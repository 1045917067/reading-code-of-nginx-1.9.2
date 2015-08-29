
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_EVENT_CONNECT_H_INCLUDED_
#define _NGX_EVENT_CONNECT_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


#define NGX_PEER_KEEPALIVE           1
#define NGX_PEER_NEXT                2
#define NGX_PEER_FAILED              4


typedef struct ngx_peer_connection_s  ngx_peer_connection_t;

//��ʹ�ó����������η�����ͨ��ʱ����ͨ���÷��������ӳ��л�ȡһ��������
typedef ngx_int_t (*ngx_event_get_peer_pt)(ngx_peer_connection_t *pc,
    void *data);
//��ʹ�ó����������η�����ͨ��ʱ��ͨ���÷�����ʹ����ϵ������ͷŸ����ӳ�
typedef void (*ngx_event_free_peer_pt)(ngx_peer_connection_t *pc, void *data,
    ngx_uint_t state);
#if (NGX_SSL)

typedef ngx_int_t (*ngx_event_set_peer_session_pt)(ngx_peer_connection_t *pc,
    void *data);
typedef void (*ngx_event_save_peer_session_pt)(ngx_peer_connection_t *pc,
    void *data);
#endif

/*
Nginx�ж����˻��������ݽṹngx_connection_t����ʾ���ӣ�������ӱ�ʾ�ǿͻ�����������ġ�Nginx�������������ܵ�TCP���ӣ����ǿ��Լ򵥳�
��Ϊ�������ӡ�ͬʱ������Щ����Ĵ�������У�Nginx����ͼ�������������η������������ӣ����Դ����������η�����ͨ�ţ���ˣ�������
������ngx_connection_t���ǲ�ͬ�ģ�Nginx������}ngx_peer_connection_t�ṹ������ʾ�������ӣ���Ȼ��ngx_peer_connection_t����������
��ngx_connection-t�ṹ��Ϊ����ʵ�ֵġ����ڽ�˵�������������и��ֶε����壬ͬʱ��Ҫע����ǣ����������Ӷ����������ⴴ���������
���ӳ��л�ȡ����9.3.3���л�˵�����ӳص��÷���
*/
struct ngx_peer_connection_s {
    /* һ����������ʵ����Ҳ��Ҫngx_connection_t�ṹ���еĴ󲿷ֳ�Ա�����ҳ������õĿ��Ƕ�������connection��Ա */
    ngx_connection_t                *connection; //����ֵ��ngx_event_connect_peer

    struct sockaddr                 *sockaddr;//Զ�˷�������socket��ַ
    socklen_t                        socklen; //sockaddr��ַ�ĳ���
    ngx_str_t                       *name; //Զ�˷����������� 

    //��ʾ������һ��Զ�˷�����ʱ����ǰ���ӳ����쳣ʧ�ܺ�������ԵĴ�����Ҳ������������ʧ�ܴ���
    ngx_uint_t                       tries; //��ֵ��ngx_http_upstream_init_xxx_peer(����ngx_http_upstream_init_round_robin_peer)
    ngx_msec_t                       start_time;//���˷������������ӵ�ʱ��ngx_http_upstream_init_request

    //ngx_event_connect_peer��ִ��
    //��ȡ���ӵķ��������ʹ�ó����ӹ��ɵ����ӳأ���ô����Ҫʵ��get����
    ngx_event_get_peer_pt            get; //��ֵ��ngx_http_upstream_init_xxx_peer(����ngx_http_upstream_init_round_robin_peer)
    ngx_event_free_peer_pt           free; //��get������Ӧ���ͷ����ӵķ���

    /*
     ���dataָ������ں������get��free������ϴ��ݲ��������ľ��庬����ʵ��get������free
     ������ģ����أ��ɲ���ngx_event_get_peer_pt��ngx_event_free_pee r_pt����ԭ���е�data����
     */
    void                            *data; //����rr�㷨,��Ӧ�ṹngx_http_upstream_rr_peer_data_t�������ռ���ngx_http_upstream_create_round_robin_peer

#if (NGX_SSL)
    ngx_event_set_peer_session_pt    set_session;
    ngx_event_save_peer_session_pt   save_session;
#endif

    ngx_addr_t                      *local; //������ַ��Ϣ

    int                              rcvbuf; //�׽��ֵĽ��ջ�������С

    ngx_log_t                       *log; //��¼��־��ngx_log_t����

    unsigned                         cached:1; //��־λ��Ϊ1ʱ��ʾ�����connection�����Ѿ�����

    /* ngx_connection_log_error_e */
    /*NGX_ERROR_IGNORE_EINVAL  ngx_connection_log_error_e
  ��9.3.1����ngx_connection_t���log_error��������ͬ�ģ��������������log_errorֻ����λ��ֻ�ܱ��4�ִ���NGX_ERROR_IGNORE_EINVAL�����޷����
     */
    unsigned                         log_error:2;
};


ngx_int_t ngx_event_connect_peer(ngx_peer_connection_t *pc);
ngx_int_t ngx_event_get_peer(ngx_peer_connection_t *pc, void *data);


#endif /* _NGX_EVENT_CONNECT_H_INCLUDED_ */
