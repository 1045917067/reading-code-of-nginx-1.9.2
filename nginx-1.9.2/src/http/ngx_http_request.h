
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HTTP_REQUEST_H_INCLUDED_
#define _NGX_HTTP_REQUEST_H_INCLUDED_


#define NGX_HTTP_MAX_URI_CHANGES           10
#define NGX_HTTP_MAX_SUBREQUESTS           200

/* must be 2^n */
#define NGX_HTTP_LC_HEADER_LEN             32


#define NGX_HTTP_DISCARD_BUFFER_SIZE       4096
#define NGX_HTTP_LINGERING_BUFFER_SIZE     4096


#define NGX_HTTP_VERSION_9                 9    //HTTP 0/9
#define NGX_HTTP_VERSION_10                1000 //HTTP 1/0
#define NGX_HTTP_VERSION_11                1001 //HTTP 1/1

/*
��Ӧ�ͻ����������е����󷽷�:
���󷽷������з���ȫΪ��д���ж��֣����������Ľ������£�
GET     �����ȡRequest-URI����ʶ����Դ
POST    ��Request-URI����ʶ����Դ�󸽼��µ�����
HEAD    �����ȡ��Request-URI����ʶ����Դ����Ӧ��Ϣ��ͷ
PUT     ����������洢һ����Դ������Request-URI��Ϊ���ʶ
DELETE  ���������ɾ��Request-URI����ʶ����Դ
TRACE   ��������������յ���������Ϣ����Ҫ���ڲ��Ի����
CONNECT ��������ʹ��
OPTIONS �����ѯ�����������ܣ����߲�ѯ����Դ��ص�ѡ�������
*/
#define NGX_HTTP_UNKNOWN                   0x0001
#define NGX_HTTP_GET                       0x0002
#define NGX_HTTP_HEAD                      0x0004
#define NGX_HTTP_POST                      0x0008
#define NGX_HTTP_PUT                       0x0010
#define NGX_HTTP_DELETE                    0x0020
#define NGX_HTTP_MKCOL                     0x0040
#define NGX_HTTP_COPY                      0x0080
#define NGX_HTTP_MOVE                      0x0100
#define NGX_HTTP_OPTIONS                   0x0200
#define NGX_HTTP_PROPFIND                  0x0400
#define NGX_HTTP_PROPPATCH                 0x0800
#define NGX_HTTP_LOCK                      0x1000
#define NGX_HTTP_UNLOCK                    0x2000
#define NGX_HTTP_PATCH                     0x4000
#define NGX_HTTP_TRACE                     0x8000

#define NGX_HTTP_CONNECTION_CLOSE          1
#define NGX_HTTP_CONNECTION_KEEP_ALIVE     2


#define NGX_NONE                           1

//����NGX_HTTP_PARSE_HEADER_DONE��ʾ��Ӧ�����е�httpͷ����������ϣ��������ٽ��յ��Ķ�����http����
#define NGX_HTTP_PARSE_HEADER_DONE         1

#define NGX_HTTP_CLIENT_ERROR              10
#define NGX_HTTP_PARSE_INVALID_METHOD      10
#define NGX_HTTP_PARSE_INVALID_REQUEST     11
#define NGX_HTTP_PARSE_INVALID_09_METHOD   12

#define NGX_HTTP_PARSE_INVALID_HEADER      13


/* unused                                  1 */
#define NGX_HTTP_SUBREQUEST_IN_MEMORY      2
#define NGX_HTTP_SUBREQUEST_WAITED         4
#define NGX_HTTP_LOG_UNSAFE                8


/*
http��Ӧ״̬���ȫ

 

http״̬���ش��� 1xx����ʱ��Ӧ��
��ʾ��ʱ��Ӧ����Ҫ�����߼���ִ�в�����״̬���롣

http״̬���ش��� ����   ˵��
100   �������� ������Ӧ������������󡣷��������ش˴����ʾ���յ�����ĵ�һ���֣����ڵȴ����ಿ�֡� 
101   ���л�Э�飩��������Ҫ��������л�Э�飬��������ȷ�ϲ�׼���л���

http״̬���ش��� 2xx ���ɹ���
��ʾ�ɹ������������״̬���롣

http״̬���ش��� ����   ˵��
200   ���ɹ���  �������ѳɹ����������� ͨ�������ʾ�������ṩ���������ҳ��
201   ���Ѵ�����  ����ɹ����ҷ������������µ���Դ��
202   ���ѽ��ܣ�  �������ѽ������󣬵���δ����
203   ������Ȩ��Ϣ��  �������ѳɹ����������󣬵����ص���Ϣ����������һ��Դ��
204   �������ݣ�  �������ɹ����������󣬵�û�з����κ����ݡ�
205   ���������ݣ��������ɹ����������󣬵�û�з����κ����ݡ�
206   ���������ݣ�  �������ɹ������˲��� GET ����

http״̬���ش��� 3xx ���ض���
��ʾҪ���������Ҫ��һ�������� ͨ������Щ״̬���������ض���

http״̬���ش��� ����   ˵��
300   ������ѡ��  ������󣬷�������ִ�ж��ֲ����� �������ɸ��������� (user agent) ѡ��һ����������ṩ�����б�������ѡ��
301   �������ƶ���  �������ҳ�������ƶ�����λ�á� ���������ش���Ӧ���� GET �� HEAD �������Ӧ��ʱ�����Զ���������ת����λ�á�
302   ����ʱ�ƶ���  ������Ŀǰ�Ӳ�ͬλ�õ���ҳ��Ӧ���󣬵�������Ӧ����ʹ��ԭ��λ���������Ժ������
303   ���鿴����λ�ã� ������Ӧ���Բ�ͬ��λ��ʹ�õ����� GET ������������Ӧʱ�����������ش˴��롣


304   ��δ�޸ģ��Դ��ϴ�������������ҳδ�޸Ĺ��� ���������ش���Ӧʱ�����᷵����ҳ���ݡ�
305   ��ʹ�ô��� ������ֻ��ʹ�ô�������������ҳ��������������ش���Ӧ������ʾ������Ӧʹ�ô���
307   ����ʱ�ض���  ������Ŀǰ�Ӳ�ͬλ�õ���ҳ��Ӧ���󣬵�������Ӧ����ʹ��ԭ��λ���������Ժ������ 
http״̬���ش��� 4xx���������
��Щ״̬�����ʾ������ܳ��������˷������Ĵ���

http״̬���ش��� ����   ˵��
400   ���������� �����������������﷨��
401   ��δ��Ȩ�� ����Ҫ�������֤��������Ҫ��¼����ҳ�����������ܷ��ش���Ӧ��
403   ����ֹ�� �������ܾ�����
404   ��δ�ҵ��� �������Ҳ����������ҳ��
405   ���������ã� ����������ָ���ķ�����
406   �������ܣ��޷�ʹ�����������������Ӧ�������ҳ��
407   ����Ҫ������Ȩ�� ��״̬������ 401��δ��Ȩ�����ƣ���ָ��������Ӧ����Ȩʹ�ô���
408   ������ʱ��  �������Ⱥ�����ʱ������ʱ��
409   ����ͻ��  ���������������ʱ������ͻ�� ��������������Ӧ�а����йس�ͻ����Ϣ��
410   ����ɾ����  ����������Դ������ɾ�����������ͻ᷵�ش���Ӧ��
411   ����Ҫ��Ч���ȣ������������ܲ�����Ч���ݳ��ȱ�ͷ�ֶε�����
412   ��δ����ǰ��������������δ���������������������õ�����һ��ǰ��������
413   ������ʵ����󣩷������޷�����������Ϊ����ʵ����󣬳����������Ĵ���������
414   ������� URI ������ ����� URI��ͨ��Ϊ��ַ���������������޷�����
415   ����֧�ֵ�ý�����ͣ�����ĸ�ʽ��������ҳ���֧�֡�
416   ������Χ������Ҫ�����ҳ���޷��ṩ����ķ�Χ����������᷵�ش�״̬���롣
417   ��δ��������ֵ��������δ����"����"�����ͷ�ֶε�Ҫ��

http״̬���ش��� 5xx������������
��Щ״̬�����ʾ�������ڳ��Դ�������ʱ�����ڲ����� ��Щ��������Ƿ���������Ĵ��󣬶������������

http״̬���ش��� ����   ˵��
500   ���������ڲ�����  ���������������޷��������
501   ����δʵʩ�� ���������߱��������Ĺ��ܡ����磬�������޷�ʶ�����󷽷�ʱ���ܻ᷵�ش˴��롣
502   ���������أ���������Ϊ���ػ���������η������յ���Ч��Ӧ��
503   �����񲻿��ã�������Ŀǰ�޷�ʹ�ã����ڳ��ػ�ͣ��ά������ ͨ������ֻ����ʱ״̬��
504   �����س�ʱ��  ��������Ϊ���ػ��������û�м�ʱ�����η������յ�����
505   ��HTTP �汾����֧�֣� ��������֧�����������õ� HTTP Э��汾�� 

һЩ������http״̬���ش���Ϊ��
200 - �������ɹ�������ҳ
404 - �������ҳ������
503 - ���񲻿���

*/

/*
http״̬���ش��� 1xx����ʱ��Ӧ��
��ʾ��ʱ��Ӧ����Ҫ�����߼���ִ�в�����״̬���롣

http״̬���ش��� ����   ˵��
100   �������� ������Ӧ������������󡣷��������ش˴����ʾ���յ�����ĵ�һ���֣����ڵȴ����ಿ�֡� 
101   ���л�Э�飩��������Ҫ��������л�Э�飬��������ȷ�ϲ�׼���л���
*/
#define NGX_HTTP_CONTINUE                  100
#define NGX_HTTP_SWITCHING_PROTOCOLS       101
#define NGX_HTTP_PROCESSING                102


/*
http״̬���ش��� 2xx ���ɹ���
��ʾ�ɹ������������״̬���롣

http״̬���ش��� ����   ˵��
200   ���ɹ���  �������ѳɹ����������� ͨ�������ʾ�������ṩ���������ҳ��
201   ���Ѵ�����  ����ɹ����ҷ������������µ���Դ��
202   ���ѽ��ܣ�  �������ѽ������󣬵���δ����
203   ������Ȩ��Ϣ��  �������ѳɹ����������󣬵����ص���Ϣ����������һ��Դ��
204   �������ݣ�  �������ɹ����������󣬵�û�з����κ����ݡ�
205   ���������ݣ��������ɹ����������󣬵�û�з����κ����ݡ�
206   ���������ݣ�  �������ɹ������˲��� GET ����
*/
#define NGX_HTTP_OK                        200
#define NGX_HTTP_CREATED                   201
#define NGX_HTTP_ACCEPTED                  202
#define NGX_HTTP_NO_CONTENT                204
#define NGX_HTTP_PARTIAL_CONTENT           206

/*
http״̬���ش��� 3xx ���ض���
��ʾҪ���������Ҫ��һ�������� ͨ������Щ״̬���������ض���

http״̬���ش��� ����   ˵��
300   ������ѡ��  ������󣬷�������ִ�ж��ֲ����� �������ɸ��������� (user agent) ѡ��һ����������ṩ�����б�������ѡ��
301   �������ƶ���  �������ҳ�������ƶ�����λ�á� ���������ش���Ӧ���� GET �� HEAD �������Ӧ��ʱ�����Զ���������ת����λ�á�
302   ����ʱ�ƶ���  ������Ŀǰ�Ӳ�ͬλ�õ���ҳ��Ӧ���󣬵�������Ӧ����ʹ��ԭ��λ���������Ժ������
303   ���鿴����λ�ã� ������Ӧ���Բ�ͬ��λ��ʹ�õ����� GET ������������Ӧʱ�����������ش˴��롣
304   ��δ�޸ģ��Դ��ϴ�������������ҳδ�޸Ĺ��� ���������ش���Ӧʱ�����᷵����ҳ���ݡ�
305   ��ʹ�ô��� ������ֻ��ʹ�ô�������������ҳ��������������ش���Ӧ������ʾ������Ӧʹ�ô���
307   ����ʱ�ض���  ������Ŀǰ�Ӳ�ͬλ�õ���ҳ��Ӧ���󣬵�������Ӧ����ʹ��ԭ��λ���������Ժ������ 

302����
HTTP/1.1 302 Moved Temporarily
Server: nginx/1.9.2
Date: Wed, 12 Feb 2025 15:21:45 GMT
Content-Type: text/html
Content-Length: 160
Connection: keep-alive
Location: http://10.10.0.103:8080  ������յ���ack���ĺ���ٴ�����http://10.10.0.103:8080

301���ģ������ض���
HTTP/1.1 301 Moved Permanently
Server: nginx/1.9.2
Date: Wed, 12 Feb 2025 15:25:58 GMT
Content-Type: text/html
Content-Length: 184
Connection: keep-alive
Location: http://10.10.0.103:8080  ������յ��ñ��ĺ��Ժ�ֻҪ�ǵ��������ĵ�ַ

*/
#define NGX_HTTP_SPECIAL_RESPONSE          300
#define NGX_HTTP_MOVED_PERMANENTLY         301
#define NGX_HTTP_MOVED_TEMPORARILY         302 //���������ظ�302��������յ��󣬻�Ѵ���������������ͻ������µ��ض����ַ
#define NGX_HTTP_SEE_OTHER                 303
#define NGX_HTTP_NOT_MODIFIED              304
#define NGX_HTTP_TEMPORARY_REDIRECT        307

/*
http״̬���ش��� 4xx���������
��Щ״̬�����ʾ������ܳ��������˷������Ĵ���

http״̬���ش��� ����   ˵��
400   ���������� �����������������﷨��
401   ��δ��Ȩ�� ����Ҫ�������֤��������Ҫ��¼����ҳ�����������ܷ��ش���Ӧ��
403   ����ֹ�� �������ܾ�����
404   ��δ�ҵ��� �������Ҳ����������ҳ��
405   ���������ã� ����������ָ���ķ�����
406   �������ܣ��޷�ʹ�����������������Ӧ�������ҳ��
407   ����Ҫ������Ȩ�� ��״̬������ 401��δ��Ȩ�����ƣ���ָ��������Ӧ����Ȩʹ�ô���
408   ������ʱ��  �������Ⱥ�����ʱ������ʱ��
409   ����ͻ��  ���������������ʱ������ͻ�� ��������������Ӧ�а����йس�ͻ����Ϣ��
410   ����ɾ����  ����������Դ������ɾ�����������ͻ᷵�ش���Ӧ��
411   ����Ҫ��Ч���ȣ������������ܲ�����Ч���ݳ��ȱ�ͷ�ֶε�����
412   ��δ����ǰ��������������δ���������������������õ�����һ��ǰ��������
413   ������ʵ����󣩷������޷�����������Ϊ����ʵ����󣬳����������Ĵ���������
414   ������� URI ������ ����� URI��ͨ��Ϊ��ַ���������������޷�����
415   ����֧�ֵ�ý�����ͣ�����ĸ�ʽ��������ҳ���֧�֡�
416   ������Χ������Ҫ�����ҳ���޷��ṩ����ķ�Χ����������᷵�ش�״̬���롣
417   ��δ��������ֵ��������δ����"����"�����ͷ�ֶε�Ҫ��
*/
#define NGX_HTTP_BAD_REQUEST               400
#define NGX_HTTP_UNAUTHORIZED              401
#define NGX_HTTP_FORBIDDEN                 403
#define NGX_HTTP_NOT_FOUND                 404
#define NGX_HTTP_NOT_ALLOWED               405
#define NGX_HTTP_REQUEST_TIME_OUT          408
#define NGX_HTTP_CONFLICT                  409
#define NGX_HTTP_LENGTH_REQUIRED           411
#define NGX_HTTP_PRECONDITION_FAILED       412
#define NGX_HTTP_REQUEST_ENTITY_TOO_LARGE  413
#define NGX_HTTP_REQUEST_URI_TOO_LARGE     414
#define NGX_HTTP_UNSUPPORTED_MEDIA_TYPE    415
#define NGX_HTTP_RANGE_NOT_SATISFIABLE     416


/* Our own HTTP codes */

/* The special code to close connection without any response */
#define NGX_HTTP_CLOSE                     444

#define NGX_HTTP_NGINX_CODES               494

#define NGX_HTTP_REQUEST_HEADER_TOO_LARGE  494

#define NGX_HTTPS_CERT_ERROR               495
#define NGX_HTTPS_NO_CERT                  496

/*
 * We use the special code for the plain HTTP requests that are sent to
 * HTTPS port to distinguish it from 4XX in an error page redirection
 */
#define NGX_HTTP_TO_HTTPS                  497

/* 498 is the canceled code for the requests with invalid host name */

/*
 * HTTP does not define the code for the case when a client closed
 * the connection while we are processing its request so we introduce
 * own code to log such situation when a client has closed the connection
 * before we even try to send the HTTP header to it
 */
#define NGX_HTTP_CLIENT_CLOSED_REQUEST     499


/*
http״̬���ش��� 5xx������������
��Щ״̬�����ʾ�������ڳ��Դ�������ʱ�����ڲ����� ��Щ��������Ƿ���������Ĵ��󣬶������������

http״̬���ش��� ����   ˵��
500   ���������ڲ�����  ���������������޷��������
501   ����δʵʩ�� ���������߱��������Ĺ��ܡ����磬�������޷�ʶ�����󷽷�ʱ���ܻ᷵�ش˴��롣
502   ���������أ���������Ϊ���ػ���������η������յ���Ч��Ӧ��
503   �����񲻿��ã�������Ŀǰ�޷�ʹ�ã����ڳ��ػ�ͣ��ά������ ͨ������ֻ����ʱ״̬��
504   �����س�ʱ��  ��������Ϊ���ػ��������û�м�ʱ�����η������յ�����
505   ��HTTP �汾����֧�֣� ��������֧�����������õ� HTTP Э��汾�� 
*/
#define NGX_HTTP_INTERNAL_SERVER_ERROR     500
#define NGX_HTTP_NOT_IMPLEMENTED           501
#define NGX_HTTP_BAD_GATEWAY               502
#define NGX_HTTP_SERVICE_UNAVAILABLE       503
#define NGX_HTTP_GATEWAY_TIME_OUT          504
#define NGX_HTTP_INSUFFICIENT_STORAGE      507

/*
�����е�ҵ�����͡��κ��¼�����ģ�鶼�����Զ�����Ҫ�ı�־λ�����buffered�ֶ���8λ��������ͬʱ��ʾ8����ͬ��ҵ�񡣵�����ģ
�����Զ���buffered��־λʱע�ⲻҪ�����ʹ�õ�ģ�鶨��ı�־λ��ͻ��Ŀǰopensslģ�鶨����һ����־λ��
    #define NGX_SSL_BUFFERED    Ox01
    
    HTTP�ٷ�ģ�鶨�������±�־λ��
    #define blGX HTTP_LOWLEVEL_BUFFERED   0xf0
    #define NGX_HTTP_WRITE_BUFFERED       0x10
    #define NGX_HTTP_GZIP_BUFFERED        0x20
    #define NGX_HTTP_SSI_BUFFERED         0x01
    #define NGX_HTTP_SUB_BUFFERED         0x02
    #define NGX_HTTP_COPY_BUFFERED        0x04
    #define NGX_HTTP_IMAGE_BUFFERED       Ox08
ͬʱ������HTTPģ����ԣ�buffered�ĵ�4λҪ���ã���ʵ�ʷ�����Ӧ��ngx_http_write_filter_module����ģ���У���4λ��־λΪ1��ζ��
Nginx��һֱ��Ϊ��HTTPģ�黹��Ҫ����������󣬱���ȴ�HTTPģ�齫��4λȫ��Ϊ0�Ż������������󡣼���4λ�ĺ����£�
    #define NGX_LOWLEVEL_BUFFERED  OxOf
 */
//һ����Щ��ngx_connection_s�е�buffered��־��ʹ��
#define NGX_HTTP_LOWLEVEL_BUFFERED         0xf0
//����HTTP���out�������л�����Ӧ�ȴ����͡� ngx_http_request_t->out�е������Ƿ�����ϣ��ο�ngx_http_write_filter
#define NGX_HTTP_WRITE_BUFFERED            0x10 
#define NGX_HTTP_GZIP_BUFFERED             0x20
#define NGX_HTTP_SSI_BUFFERED              0x01
#define NGX_HTTP_SUB_BUFFERED              0x02
#define NGX_HTTP_COPY_BUFFERED             0x04


typedef enum {
    NGX_HTTP_INITING_REQUEST_STATE = 0,
    NGX_HTTP_READING_REQUEST_STATE,
    NGX_HTTP_PROCESS_REQUEST_STATE,

    NGX_HTTP_CONNECT_UPSTREAM_STATE,
    NGX_HTTP_WRITING_UPSTREAM_STATE,
    NGX_HTTP_READING_UPSTREAM_STATE,

    NGX_HTTP_WRITING_REQUEST_STATE,
    NGX_HTTP_LINGERING_CLOSE_STATE,
    NGX_HTTP_KEEPALIVE_STATE
} ngx_http_state_e;


typedef struct { //��ngx_http_headers_in
    ngx_str_t                         name; //ƥ��ͷ����name:value�е�name����ngx_http_process_request_headers
    ngx_uint_t                        offset;
    //handler�����������ֱ�Ϊ(r, h, hh->offset):rΪ��Ӧ����������h�洢Ϊͷ����key:value(��:Content-Type: text/html)ֵ��
    //hh->offset��ngx_http_headers_in�г�Ա�Ķ�Ӧoffset(�������д���host����offset=offsetof(ngx_http_headers_in_t, host))
    ngx_http_header_handler_pt        handler; //ngx_http_process_request_headers��ִ�� 
} ngx_http_header_t;


typedef struct {
    ngx_str_t                         name;
    ngx_uint_t                        offset;
} ngx_http_header_out_t;

//���͵�headers_in��洢�Ѿ���������HTTPͷ�����������ngx_http_headers_in_t�ṹ���еĳ�Ա�� ngx_http_request_s���������а����ýṹ
//�ο�ngx_http_headers_in��ͨ���������еĻص�hander���洢��������������name:value�е�value��headers_in����Ӧ��Ա�У���ngx_http_process_request_headers
typedef struct { 
    /*
    ��ȡHTTPͷ��ʱ��ֱ��ʹ��r->headers_in����Ӧ��Ա�Ϳ����ˡ��������˵��һ�����ͨ������headers�����ȡ��RFC2616��׼��HTTPͷ����
    �����Ȼع�һ��ngx_list_t�����ngx_table_elt_t�ṹ����÷���headers��һ��ngx_list_t�������洢�Ž�����������HTTPͷ����������
    ��Ԫ�ض���ngx_table_elt_t���͡����波����һ���û��������ҵ���Rpc-Description��ͷ���������ж���ֵ�Ƿ�Ϊ��uploadFile�����پ���
    �����ķ�������Ϊ���������¡�
    ngx_list_part_t *part = &r->headers_in.headers.part;
    ngx_table_elt_t *header = part->elts;
    
    //��ʼ��������
    for (i = 0; ; i++) {
     //�ж��Ƿ񵽴������е�ǰ����Ľ�β��
     if (i >= part->nelts) {
      //�Ƿ�����һ����������Ԫ��
      if (part->next == NULL) {
       break;
      }
    
       part����Ϊnext��������һ���������飻headerҲָ����һ������������׵�ַ��i����Ϊ0ʱ����ʾ��ͷ��ʼ�����µ���������
      part = part->next;
      header = part->elts;
      i = 0;
     }
    
     //hashΪ0ʱ��ʾ���ǺϷ���ͷ��
     if (header[i].hash == 0) {
      continue;
     }
    
     �жϵ�ǰ��ͷ���Ƿ��ǡ�Rpc-Description���������Ҫ���Դ�Сд����Ӧ������header[i].lowcase_key����header[i].key.data��Ȼ��Ƚ��ַ���
     if (0 == ngx_strncasecmp(header[i].key.data,
       (u_char*) "Rpc-Description",
       header[i].key.len))
     {
      //�ж����HTTPͷ����ֵ�Ƿ��ǡ�uploadFile��
      if (0 == ngx_strncmp(header[i].value.data,
        "uploadFile",
        header[i].value.len))
      {
       //�ҵ�����ȷ��ͷ������������ִ��
      }
     }
    }
    
    ���ڳ�����HTTPͷ����ֱ�ӻ�ȡr->headers_in���Ѿ���HTTP��ܽ������ĳ�Ա���ɣ������ڲ�������HTTPͷ������Ҫ����r->headers_in.headers������ܻ�á�
*/

    /*���н�������HTTPͷ������headers�����У�����ʹ��3.2.3���н��ܵı�������ķ�������ȡ���е�HTTPͷ����ע�⣬����headers�����
    ÿһ��Ԫ�ض���ngx_table_elt_t��Ա*/ //��ngx_http_headers_in��ȡ������洢���������У������еĳ�Ա��������ĸ���ngx_table_elt_t��Ա
    ngx_list_t                        headers; //��ngx_http_process_request_line��ʼ��list�ռ�  ngx_http_process_request_headers�д洢��������������value��key

    /*
      HTTP ͷ������
     
     1. Accept������WEB�������Լ�����ʲô�������ͣ�* / * ��ʾ�κ����ͣ�type/ * ��ʾ�������µ����������ͣ�type/sub-type��
      
     2. Accept-Charset��   ����������Լ����յ��ַ���
        Accept-Encoding��  ����������Լ����յı��뷽����ͨ��ָ��ѹ���������Ƿ�֧��ѹ����֧��ʲôѹ������  ��gzip��deflate��
        Accept-Language��������������Լ����յ��������Ը��ַ������������������ԣ������ж����ַ���������big5��gb2312��gbk�ȵȡ�
      
     3. Accept-Ranges��WEB�����������Լ��Ƿ���ܻ�ȡ��ĳ��ʵ���һ���֣������ļ���һ���֣�������bytes����ʾ���ܣ�none����ʾ�����ܡ�
      
     4. Age����������������Լ������ʵ��ȥ��Ӧ����ʱ���ø�ͷ��������ʵ��Ӳ��������ھ����೤ʱ���ˡ�
      
     5. Authorization�����ͻ��˽��յ�����WEB�������� WWW-Authenticate ��Ӧʱ����ͷ������Ӧ�Լ��������֤��Ϣ��WEB��������
      
     6. Cache-Control������no-cache����Ҫ�����ʵ�壬Ҫ�����ڴ�WEB������ȥȡ��
                              max-age����ֻ���� Age ֵС�� max-age ֵ������û�й��ڵĶ���
                              max-stale�������Խ��ܹ�ȥ�Ķ��󣬵��ǹ���ʱ�����С�� 
                                                 max-stale ֵ��
                              min-fresh�������������������ڴ����䵱ǰ Age �� min-fresh ֵ֮�͵Ļ������
                       ��Ӧ��public(������ Cached ���ݻ�Ӧ�κ��û�)
                               private��ֻ���û������ݻ�Ӧ��ǰ��������ݵ��Ǹ��û���
                               no-cache�����Ի��棬����ֻ���ڸ�WEB��������֤������Ч�󣬲��ܷ��ظ��ͻ��ˣ�
                               max-age��������Ӧ�����Ķ���Ĺ���ʱ�䣩
                               ALL:  no-store���������棩
      
     7. Connection������close������WEB���������ߴ��������������ɱ����������Ӧ
                                                       �󣬶Ͽ����ӣ���Ҫ�ȴ��������ӵĺ��������ˣ���
                                      keepalive������WEB���������ߴ��������������ɱ��������
                                                              ��Ӧ�󣬱������ӣ��ȴ��������ӵĺ������󣩡�
                            ��Ӧ��close�������Ѿ��رգ���
                                      keepalive�����ӱ����ţ��ڵȴ��������ӵĺ������󣩡�
        Keep-Alive�������������󱣳����ӣ����ͷ������ϣ�� WEB ����������
                           ���Ӷ೤ʱ�䣨�룩��
                           ���磺Keep-Alive��300
      
     8. Content-Encoding��WEB�����������Լ�ʹ����ʲôѹ��������gzip��deflate��ѹ����Ӧ�еĶ��� 
                                      ���磺Content-Encoding��gzip                   
        Content-Language��WEB ����������������Լ���Ӧ�Ķ�������ԡ�
        Content-Length��    WEB ����������������Լ���Ӧ�Ķ���ĳ��ȡ�
                                     ���磺Content-Length: 26012
        Content-Range��    WEB ��������������Ӧ�����Ĳ��ֶ���Ϊ����������ĸ����֡�
                                     ���磺Content-Range: bytes 21010-47021/47022
        Content-Type��      WEB ����������������Լ���Ӧ�Ķ�������͡�
                                     ���磺Content-Type��application/xml
      
     9. ETag������һ�����󣨱���URL���ı�־ֵ����һ��������ԣ�����һ�� html �ļ���
                   ������޸��ˣ��� Etag Ҳ����޸ģ� ���ԣ�ETag �����ø� Last-Modified ��
                   ���ò�࣬��Ҫ�� WEB ������ �ж�һ�������Ƿ�ı��ˡ�
                   ����ǰһ������ĳ�� html �ļ�ʱ��������� ETag�����������������ļ�ʱ�� 
                   ������ͻ����ǰ��õ� ETag ֵ���͸�  WEB ��������Ȼ�� WEB ������
                   ������ ETag �����ļ��ĵ�ǰ ETag ���жԱȣ�Ȼ���֪������ļ�
                   ��û�иı��ˡ�
              
     10. Expired��WEB������������ʵ�彫��ʲôʱ����ڣ����ڹ����˵Ķ���ֻ����
                  ��WEB��������֤������Ч�Ժ󣬲���������Ӧ�ͻ������� HTTP/1.0 ��ͷ����
                  ���磺Expires��Sat, 23 May 2009 10:02:12 GMT
      
     11. Host���ͻ���ָ���Լ�����ʵ�WEB������������/IP ��ַ�Ͷ˿ںš�
                     ���磺Host��rss.sina.com.cn
      
     12. If-Match���������� ETag û�иı䣬��ʵҲ����ζ������û�иı䣬��ִ������Ķ�����
         If-None-Match���������� ETag �ı��ˣ���ʵҲ����ζ������Ҳ�ı��ˣ���ִ������Ķ�����
      
     13. If-Modified-Since���������Ķ����ڸ�ͷ��ָ����ʱ��֮���޸��ˣ���ִ������
                            �Ķ��������緵�ض��󣩣����򷵻ش���304������������ö���û���޸ġ�
                            ���磺If-Modified-Since��Thu, 10 Apr 2008 09:14:42 GMT
         If-Unmodified-Since���������Ķ����ڸ�ͷ��ָ����ʱ��֮��û�޸Ĺ�����ִ������Ķ��������緵�ض��󣩡�
      
     14. If-Range����������� WEB �����������������Ķ���û�иı䣬�Ͱ���ȱ�ٵĲ���
                    ���ң��������ı��ˣ��Ͱ�����������ҡ� �����ͨ�������������� 
                    ETag ���� �Լ���֪��������޸�ʱ��� WEB �������������ж϶����Ƿ�
                    �ı��ˡ����Ǹ� Range ͷ��һ��ʹ�á�
      
     15. Last-Modified��WEB ��������Ϊ���������޸�ʱ�䣬�����ļ�������޸�ʱ�䣬
                        ��̬ҳ���������ʱ��ȵȡ����磺Last-Modified��Tue, 06 May 2008 02:42:43 GMT
      
     16. Location��WEB �������������������ͼ���ʵĶ����Ѿ����Ƶ����λ���ˣ�����ͷ��ָ����λ��ȥȡ��
                             ���磺Location��http://i0.sinaimg.cn/dy/deco/2008/0528/sinahome_0803_ws_005_text_0.gif
      
     17. Pramga����Ҫʹ�� Pramga: no-cache���൱�� Cache-Control�� no-cache�����磺Pragma��no-cache
      
     18. Proxy-Authenticate�� �����������Ӧ�������Ҫ�����ṩ���������֤��Ϣ��
           Proxy-Authorization���������Ӧ����������������֤�����ṩ�Լ��������Ϣ��
      
     19. Range������������� Flashget ���߳�����ʱ������ WEB �������Լ���ȡ������Ĳ��֡�
                         ���磺Range: bytes=1173546-
      
     20. Referer��������� WEB �����������Լ��Ǵ��ĸ� ��ҳ/URL ���/��� ��ǰ�����е���ַ/URL��
                        ���磺Referer��http://www.sina.com/
      
     21. Server: WEB �����������Լ���ʲô������汾����Ϣ�����磺Server��Apache/2.0.61 (Unix)
      
     22. User-Agent: ����������Լ�����ݣ����������������
                             ���磺User-Agent��Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN;   
                                       rv:1.8.1.14) Gecko/20080404 Firefox/2.0.0.14
      
     23. Transfer-Encoding: WEB �����������Լ��Ա���Ӧ��Ϣ�壨������Ϣ������Ķ������������ı��룬�����Ƿ�ֿ飨chunked����
                            ���磺Transfer-Encoding: chunked
      
     24. Vary: WEB�������ø�ͷ�������ݸ��� Cache ����������ʲô�����²����ñ���Ӧ
                      �����صĶ�����Ӧ����������
                      ����ԴWEB�������ڽӵ���һ��������Ϣʱ������Ӧ��Ϣ��ͷ��Ϊ��
                      Content-Encoding: gzip; Vary: Content-Encoding  ��ô Cache ���������������
                      ������Ϣ��ͷ��������� Accept-Encoding���Ƿ����ǰ��Ӧ�� Vary ͷ��ֵ
                      һ�£����Ƿ�ʹ����ͬ�����ݱ��뷽���������Ϳ��Է�ֹ Cache ���������Լ�
                      Cache ����ѹ�����ʵ����Ӧ�����߱���ѹ�������������
                      ���磺Vary��Accept-Encoding
      
     25. Via�� �г��ӿͻ��˵� OCS �����෴�������Ӧ��������Щ�����������������
             ʲôЭ�飨�Ͱ汾�����͵����󡣵��ͻ������󵽴��һ�����������ʱ���÷����������Լ���������������
             ��� Via ͷ�����������Լ��������Ϣ������һ����������� �յ���һ�����������������ʱ�������Լ���
             �����������渴��ǰһ������������������Viaͷ���������Լ��������Ϣ�ӵ����棬 �Դ����ƣ��� OCS 
             �յ����һ�����������������ʱ����� Via ͷ������֪����������������·�ɡ�
             ���磺Via��1.0 236-81.D07071953.sina.com.cn:80 (squid/2.6.STABLE13)

    */
    /*����ÿ��ngx_table_elt_t��Ա����RFC1616�淶�ж����HTTPͷ���� ����ʵ�ʶ�ָ��headers�����е���Ӧ��Ա��ע�⣬
    ������ΪNULL��ָ��ʱ����ʾû�н�������Ӧ��HTTPͷ��*/ //server��hostָ������һ��������ͷ����Я����hostͷ��
    ngx_table_elt_t                  *host; //http1.0���ϱ������hostͷ���У���ngx_http_process_request_header
    ngx_table_elt_t                  *connection;
    ngx_table_elt_t                  *if_modified_since;
    ngx_table_elt_t                  *if_unmodified_since;
    ngx_table_elt_t                  *if_match;
    ngx_table_elt_t                  *if_none_match;
    ngx_table_elt_t                  *user_agent;
    ngx_table_elt_t                  *referer;
    ngx_table_elt_t                  *content_length;
    ngx_table_elt_t                  *content_type;

    ngx_table_elt_t                  *range;
    ngx_table_elt_t                  *if_range;

    ngx_table_elt_t                  *transfer_encoding; //Transfer-Encoding
    ngx_table_elt_t                  *expect;
    ngx_table_elt_t                  *upgrade;

#if (NGX_HTTP_GZIP)
    ngx_table_elt_t                  *accept_encoding;
    ngx_table_elt_t                  *via;
#endif

    ngx_table_elt_t                  *authorization;
    //ֻ����connection_type == NGX_HTTP_CONNECTION_KEEP_ALIVE������²���Ч    Connection=keep-aliveʱ����Ч
    ngx_table_elt_t                  *keep_alive; //��ֵngx_http_process_header_line

#if (NGX_HTTP_X_FORWARDED_FOR)
    ngx_array_t                       x_forwarded_for;
#endif

#if (NGX_HTTP_REALIP)
    ngx_table_elt_t                  *x_real_ip;
#endif

#if (NGX_HTTP_HEADERS)
    ngx_table_elt_t                  *accept;
    ngx_table_elt_t                  *accept_language;
#endif

#if (NGX_HTTP_DAV)
    ngx_table_elt_t                  *depth;
    ngx_table_elt_t                  *destination;
    ngx_table_elt_t                  *overwrite;
    ngx_table_elt_t                  *date;
#endif

    /* user��passwd��ֻ��ngx_http_auth_basic_module�Ż��õ��ĳ�Ա��������Ժ��� */
    ngx_str_t                         user;
    ngx_str_t                         passwd;

    /*cookies����ngx_array_t����洢�ģ������Ȳ�����������ݽṹ������Ȥ�Ļ�����ֱ������7.3���˽�ngx_array_t������÷�*/
    ngx_array_t                       cookies;
    //server��hostָ������һ��������ͷ����Я����hostͷ��
    ngx_str_t                         server;//server����   ngx_http_process_host

    /* �ڶ��������ʱ��(��ngx_http_read_discarded_request_body)��headers_in��Ա���content_length_n�����������content-lengthͷ������ÿ����һ���ְ��壬�ͻ���content_length_n����
    �м�ȥ��Ӧ�Ĵ�С����ˣ�content_length_n��ʾ����Ҫ�����İ��峤�ȣ��������ȼ�������content_length_n��Ա��������Ѿ�����0�����ʾ�Ѿ����յ������İ��� 
    */
    //������ͷ���к�ͨ��ngx_http_process_request_header�����ٿռ�Ӷ����洢�������е����ݣ���ʾ�������Ĵ�С�����Ϊ-1��ʾ�����в�������
    off_t                             content_length_n; //����ngx_table_elt_t *content_length�������HTTP�����С  Ĭ�ϸ�ֵ-1
    time_t                            keep_alive_n; //ͷ����Keep-Alive:�������� Connection=keep-aliveʱ����Ч  ��ֵngx_http_process_request_header

    /*HTTP�������ͣ�����ȡֵ��Χ��0��NGX_http_CONNECTION_CLOSE����NGX_HTTP_CONNECTION_KEEP_ALIVE*/
    unsigned                          connection_type:2; //NGX_HTTP_CONNECTION_KEEP_ALIVE�ȣ���ʾ������ ������

/*����7����־λ��HTTP��ܸ�������������ġ�useragent��ͷ�������ǿ������ж�����������ͣ�ֵΪ1ʱ��ʾ����Ӧ�����������������ֵΪ0ʱ���෴*/
    unsigned                          chunked:1;//Transfer-Encoding:chunked��ʱ����1
    unsigned                          msie:1; //IE Internet Explorer�����IE
    unsigned                          msie6:1; //
    unsigned                          opera:1; //Opera ���������ѡ����١���ȫ������Ų��������ȫ��
    unsigned                          gecko:1; //Gecko��һ�����ɼ� ����Դ���롢��C++��д��ҳ �Ű����棬ĿǰΪMozilla Firefox ��ҳ�������
    unsigned                          chrome:1; //Google Chrome��һ����١����Ұ�ȫ�����������
    unsigned                          safari:1; //Safari��ƻ����˾�з��������������_
    unsigned                          konqueror:1;  //Konqueror v4.8.2. ��ǰ����ٵ������֮һ
} ngx_http_headers_in_t;


/*
����headers����������Զ����HTTPͷ��ʱ�����Բο�3.2.3����ngx_list_push��ʹ�÷�����������һ���򵥵����ӣ�������ʾ��
ngx_table_elt_t* h = ngx_list_push(&r->headers_out.headers);
if (h == NULL) {
 return NGX_ERROR;
}

h->hash = 1;
h->key.len = sizeof("TestHead") - 1;
h->key.data = (u_char *) "TestHead";
h->value.len = sizeof("TestValue") - 1;
h->value.data = (u_char *) "TestValue";

������������Ӧ������һ��HTTPͷ����
TestHead: TestValud\r\n

������͵���һ��������HTTP�������Ӧ����ʱ�Ϳ���ֱ�ӽ��������ˣ����磬��ngx_http_mytest_handler�����У�ֱ����
ngx_http_send_header����ִ�к��䷵��ֵreturn���ɣ���

ע�⡡ngx_http_send_header���������ȵ������е�HTTP����ģ�鹲ͬ����headers_out�ж����HTTP��Ӧͷ����ȫ��������
�Ϻ�Ż����л�ΪTCP�ַ������͵��ͻ��ˣ�������̿ɲμ�11.9.1��
*/
typedef struct { //������ngx_http_request_s�ṹ��
    //��������˺��(����fastcgi��PHP������),����洢���Ǻ�˷��������ص�һ��һ�е�ͷ������Ϣ,��ֵ��ngx_http_upstream_process_headers->ngx_http_upstream_copy_header_line
    ngx_list_t                        headers;//�����͵�HTTPͷ��������headers_in�е�headers��Ա����  

    ngx_uint_t                        status;/*��Ӧ�е�״ֵ̬����200��ʾ�ɹ����������ʹ��3.6.1���н��ܹ��ĸ����꣬��NGX_HTTP_OK */
    ngx_str_t                         status_line;//��Ӧ��״̬�У��硰HTTP/1.1 201 CREATED��

/*
      HTTP ͷ������
     
     1. Accept������WEB�������Լ�����ʲô�������ͣ�* / * ��ʾ�κ����ͣ�type/ * ��ʾ�������µ����������ͣ�type/sub-type��
      
     2. Accept-Charset��   ����������Լ����յ��ַ���
        Accept-Encoding��  ����������Լ����յı��뷽����ͨ��ָ��ѹ���������Ƿ�֧��ѹ����֧��ʲôѹ������  ��gzip��deflate��
        Accept-Language��������������Լ����յ��������Ը��ַ������������������ԣ������ж����ַ���������big5��gb2312��gbk�ȵȡ�
      
     3. Accept-Ranges��WEB�����������Լ��Ƿ���ܻ�ȡ��ĳ��ʵ���һ���֣������ļ���һ���֣�������bytes����ʾ���ܣ�none����ʾ�����ܡ�
      
     4. Age����������������Լ������ʵ��ȥ��Ӧ����ʱ���ø�ͷ��������ʵ��Ӳ��������ھ����೤ʱ���ˡ�
      
     5. Authorization�����ͻ��˽��յ�����WEB�������� WWW-Authenticate ��Ӧʱ����ͷ������Ӧ�Լ��������֤��Ϣ��WEB��������
      
     6. Cache-Control������no-cache����Ҫ�����ʵ�壬Ҫ�����ڴ�WEB������ȥȡ��
                              max-age����ֻ���� Age ֵС�� max-age ֵ������û�й��ڵĶ���
                              max-stale�������Խ��ܹ�ȥ�Ķ��󣬵��ǹ���ʱ�����С�� 
                                                 max-stale ֵ��
                              min-fresh�������������������ڴ����䵱ǰ Age �� min-fresh ֵ֮�͵Ļ������
                       ��Ӧ��public(������ Cached ���ݻ�Ӧ�κ��û�)
                               private��ֻ���û������ݻ�Ӧ��ǰ��������ݵ��Ǹ��û���
                               no-cache�����Ի��棬����ֻ���ڸ�WEB��������֤������Ч�󣬲��ܷ��ظ��ͻ��ˣ�
                               max-age��������Ӧ�����Ķ���Ĺ���ʱ�䣩
                               ALL:  no-store���������棩
      
     7. Connection������close������WEB���������ߴ��������������ɱ����������Ӧ
                                                       �󣬶Ͽ����ӣ���Ҫ�ȴ��������ӵĺ��������ˣ���
                                      keepalive������WEB���������ߴ��������������ɱ��������
                                                              ��Ӧ�󣬱������ӣ��ȴ��������ӵĺ������󣩡�
                            ��Ӧ��close�������Ѿ��رգ���
                                      keepalive�����ӱ����ţ��ڵȴ��������ӵĺ������󣩡�
        Keep-Alive�������������󱣳����ӣ����ͷ������ϣ�� WEB ����������
                           ���Ӷ೤ʱ�䣨�룩��
                           ���磺Keep-Alive��300
      
     8. Content-Encoding��WEB�����������Լ�ʹ����ʲôѹ��������gzip��deflate��ѹ����Ӧ�еĶ��� 
                                      ���磺Content-Encoding��gzip                   
        Content-Language��WEB ����������������Լ���Ӧ�Ķ�������ԡ�
        Content-Length��    WEB ����������������Լ���Ӧ�Ķ���ĳ��ȡ�
                                     ���磺Content-Length: 26012
        Content-Range��    WEB ��������������Ӧ�����Ĳ��ֶ���Ϊ����������ĸ����֡�
                                     ���磺Content-Range: bytes 21010-47021/47022
        Content-Type��      WEB ����������������Լ���Ӧ�Ķ�������͡�
                                     ���磺Content-Type��application/xml
      
     9. ETag������һ�����󣨱���URL���ı�־ֵ����һ��������ԣ�����һ�� html �ļ���
                   ������޸��ˣ��� Etag Ҳ����޸ģ� ���ԣ�ETag �����ø� Last-Modified ��
                   ���ò�࣬��Ҫ�� WEB ������ �ж�һ�������Ƿ�ı��ˡ�
                   ����ǰһ������ĳ�� html �ļ�ʱ��������� ETag�����������������ļ�ʱ�� 
                   ������ͻ����ǰ��õ� ETag ֵ���͸�  WEB ��������Ȼ�� WEB ������
                   ������ ETag �����ļ��ĵ�ǰ ETag ���жԱȣ�Ȼ���֪������ļ�
                   ��û�иı��ˡ�
              
     10. Expired��WEB������������ʵ�彫��ʲôʱ����ڣ����ڹ����˵Ķ���ֻ����
                  ��WEB��������֤������Ч�Ժ󣬲���������Ӧ�ͻ������� HTTP/1.0 ��ͷ����
                  ���磺Expires��Sat, 23 May 2009 10:02:12 GMT
      
     11. Host���ͻ���ָ���Լ�����ʵ�WEB������������/IP ��ַ�Ͷ˿ںš�
                     ���磺Host��rss.sina.com.cn
      
     12. If-Match���������� ETag û�иı䣬��ʵҲ����ζ������û�иı䣬��ִ������Ķ�����
         If-None-Match���������� ETag �ı��ˣ���ʵҲ����ζ������Ҳ�ı��ˣ���ִ������Ķ�����
      
     13. If-Modified-Since���������Ķ����ڸ�ͷ��ָ����ʱ��֮���޸��ˣ���ִ������
                            �Ķ��������緵�ض��󣩣����򷵻ش���304������������ö���û���޸ġ�
                            ���磺If-Modified-Since��Thu, 10 Apr 2008 09:14:42 GMT
         If-Unmodified-Since���������Ķ����ڸ�ͷ��ָ����ʱ��֮��û�޸Ĺ�����ִ������Ķ��������緵�ض��󣩡�
      
     14. If-Range����������� WEB �����������������Ķ���û�иı䣬�Ͱ���ȱ�ٵĲ���
                    ���ң��������ı��ˣ��Ͱ�����������ҡ� �����ͨ�������������� 
                    ETag ���� �Լ���֪��������޸�ʱ��� WEB �������������ж϶����Ƿ�
                    �ı��ˡ����Ǹ� Range ͷ��һ��ʹ�á�
      
     15. Last-Modified��WEB ��������Ϊ���������޸�ʱ�䣬�����ļ�������޸�ʱ�䣬
                        ��̬ҳ���������ʱ��ȵȡ����磺Last-Modified��Tue, 06 May 2008 02:42:43 GMT
      
     16. Location��WEB �������������������ͼ���ʵĶ����Ѿ����Ƶ����λ���ˣ�����ͷ��ָ����λ��ȥȡ��
                             ���磺Location��http://i0.sinaimg.cn/dy/deco/2008/0528/sinahome_0803_ws_005_text_0.gif
      
     17. Pramga����Ҫʹ�� Pramga: no-cache���൱�� Cache-Control�� no-cache�����磺Pragma��no-cache
      
     18. Proxy-Authenticate�� �����������Ӧ�������Ҫ�����ṩ���������֤��Ϣ��
           Proxy-Authorization���������Ӧ����������������֤�����ṩ�Լ��������Ϣ��
      
     19. Range������������� Flashget ���߳�����ʱ������ WEB �������Լ���ȡ������Ĳ��֡�
                         ���磺Range: bytes=1173546-
      
     20. Referer��������� WEB �����������Լ��Ǵ��ĸ� ��ҳ/URL ���/��� ��ǰ�����е���ַ/URL��
                        ���磺Referer��http://www.sina.com/
      
     21. Server: WEB �����������Լ���ʲô������汾����Ϣ�����磺Server��Apache/2.0.61 (Unix)
      
     22. User-Agent: ����������Լ�����ݣ����������������
                             ���磺User-Agent��Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN;   
                                       rv:1.8.1.14) Gecko/20080404 Firefox/2.0.0.14
      
     23. Transfer-Encoding: WEB �����������Լ��Ա���Ӧ��Ϣ�壨������Ϣ������Ķ������������ı��룬�����Ƿ�ֿ飨chunked����
                            ���磺Transfer-Encoding: chunked
      
     24. Vary: WEB�������ø�ͷ�������ݸ��� Cache ����������ʲô�����²����ñ���Ӧ
                      �����صĶ�����Ӧ����������
                      ����ԴWEB�������ڽӵ���һ��������Ϣʱ������Ӧ��Ϣ��ͷ��Ϊ��
                      Content-Encoding: gzip; Vary: Content-Encoding  ��ô Cache ���������������
                      ������Ϣ��ͷ��������� Accept-Encoding���Ƿ����ǰ��Ӧ�� Vary ͷ��ֵ
                      һ�£����Ƿ�ʹ����ͬ�����ݱ��뷽���������Ϳ��Է�ֹ Cache ���������Լ�
                      Cache ����ѹ�����ʵ����Ӧ�����߱���ѹ�������������
                      ���磺Vary��Accept-Encoding
      
     25. Via�� �г��ӿͻ��˵� OCS �����෴�������Ӧ��������Щ�����������������
             ʲôЭ�飨�Ͱ汾�����͵����󡣵��ͻ������󵽴��һ�����������ʱ���÷����������Լ���������������
             ��� Via ͷ�����������Լ��������Ϣ������һ����������� �յ���һ�����������������ʱ�������Լ���
             �����������渴��ǰһ������������������Viaͷ���������Լ��������Ϣ�ӵ����棬 �Դ����ƣ��� OCS 
             �յ����һ�����������������ʱ����� Via ͷ������֪����������������·�ɡ�
             ���磺Via��1.0 236-81.D07071953.sina.com.cn:80 (squid/2.6.STABLE13)

    */
    /*����ÿ��ngx_table_elt_t��Ա����RFC1616�淶�ж����HTTPͷ���� ����ʵ�ʶ�ָ��headers�����е���Ӧ��Ա��ע�⣬
    ������ΪNULL��ָ��ʱ����ʾû�н�������Ӧ��HTTPͷ��*/

/*���³�Ա������ngx_table_elt_t������RFC1616�淶�ж����HTTPͷ�������ú�ngx_http_header_filter_module����ģ����԰����Ǽӵ������͵��������*/
    ngx_table_elt_t                  *server;
    ngx_table_elt_t                  *date;
    ngx_table_elt_t                  *content_length;
    ngx_table_elt_t                  *content_encoding;
    /* rewrite ^(.*)$ http://$1.mp4 break; ���uriΪhttp://10.135.0.1/aaa,��location�д洢����aaa.mp4 */
    ngx_table_elt_t                  *location; //�洢redirectʱ��ת������ַ���ֵ����ngx_http_script_regex_end_code
    ngx_table_elt_t                  *refresh;
    ngx_table_elt_t                  *last_modified;
    ngx_table_elt_t                  *content_range;
    ngx_table_elt_t                  *accept_ranges;
    ngx_table_elt_t                  *www_authenticate;
    ngx_table_elt_t                  *expires;
    ngx_table_elt_t                  *etag;

    ngx_str_t                        *override_charset;

/*���Ե���ngx_http_set_content_type(r)����������������Content-Typeͷ����������������URI�е��ļ���չ������Ӧ��mime.type������Content-Typeֵ,ȡֵ��:image/jpeg*/
    size_t                            content_type_len;
    ngx_str_t                         content_type;
    ngx_str_t                         charset; //�Ǵ�content_type�н��������ģ���ngx_http_upstream_copy_content_type
    u_char                           *content_type_lowcase;
    ngx_uint_t                        content_type_hash;

    ngx_array_t                       cache_control;
    /*������ָ����content_length_n�󣬲����ٴε�ngx_table_elt_t *content_ length��������Ӧ����*/
    off_t                             content_length_n; //�����ʾӦ����ĳ���
    time_t                            date_time;
    time_t                            last_modified_time;
} ngx_http_headers_out_t;

//��ngx_http_read_client_request_body�У����������Ϻ��ִ�лص�����ngx_http_client_body_handler_pt
/*
���У��в���ngx_http_request_t *r������������Ϣ�����Դ�r�л�á��������Զ���һ������void func(ngx_http_request_t *r)����Nginx���������ʱ��������
���⣬����������Ҳ����д����������У����磺
void ngx_http_mytest_body_handler(ngx_http_request_t *r)
{
 ��
}
*/
typedef void (*ngx_http_client_body_handler_pt)(ngx_http_request_t *r);
//���ڱ���HTTP����Ľṹ��ngx_http_request_body_t
/*
���ngx_http_request_body_t�ṹ��ʹ���ڱ����������ngx_http_request_t�ṹ��
��request_body��Ա�У�����HTTP�������Χ����������ݽṹ���еġ�
*/
typedef struct {//��ngx_http_read_client_request_body�з���洢�ռ�
    //��ȡ�ͻ����弴ʹ�Ǵ�����ʱ�ļ��У������а����ȡ��Ϻ�(ngx_http_do_read_client_request_body)�����ǻ���r->request_body->bufsָ���ļ��е����ƫ���ڴ��ַ
    ngx_temp_file_t                  *temp_file; //���HTTP�������ʱ�ļ�  ngx_http_write_request_body����ռ�  //�������"client_body_in_file_only" on | clean ��ʾ����洢�ڴ����ļ���
    /* ����HTTP����Ļ�����������������Ҫȫ��������ڴ���ʱ�����һ��ngx_buf_t�������޷�����꣬��ʱ����Ҫʹ��ngx_chain_t��������� */
    //�ÿռ��Ϸ���ĸ����ڴ���Ϣ�����ջ����ngx_free_chain��֮ǰ������ڴ�����poll->chain�У��Ӷ���poll��ͬһ�����ͷ���Դ��
    //��ngx_http_write_request_body�л��bufs�����е�����ngx_buf_t�ڵ���Ϣ����ĸ���ָ��ָ�����������(�������ж�ȡ��)д�뵽��ʱ�ļ�temp_file�У�Ȼ�����Щngx_buf_t��bufs��ɾ����
    //ͨ��ngx_free_chain���뵽poll->chain����ngx_http_write_request_body��bufs�е�����д����ʱ�ļ��󣬻��bufs(ngx_chain_t)�ڵ����r->pool->chain��
    
    //��ȡ�ͻ����弴ʹ�Ǵ�����ʱ�ļ��У������а����ȡ��Ϻ�(ngx_http_do_read_client_request_body)�����ǻ���r->request_body->bufsָ���ļ��е����ƫ���ڴ��ַ
    ngx_chain_t                      *bufs;//������������ʱ�ļ��У���bufsָ���ngx_chain_t�еĸ���ָ��ָ���ļ��е����ƫ��//��ngx_http_read_client_request_body�и�ֵ
    //��buf�е�����������(buf->end = buf->last)���ͻ�д�뵽��ʱ�ļ�����߷��͵����η����������ڴ�ռ�Ϳ��Լ����洢��ȡ����������
    //ngx_http_read_client_request_body����ռ䣬Ӧ������ʱ�õģ������Ҫ��ζ�ȡ�Ż��ȡ��ϵ�ʱ��ÿ�ζ�ȡ����������ʱ�����buf�У�
    //���ջ��ǻ��ŵ������bufs�У���ngx_http_request_body_length_filter
    ngx_buf_t                        *buf;
    off_t                             rest;//����content-lengthͷ�����ѽ��յ��İ��峤�ȣ�������Ļ���Ҫ���յİ��峤��
    ngx_chain_t                      *free; //free  busy ��bufs����Ĺ�ϵ���Բο�ngx_http_request_body_length_filter
    ngx_chain_t                      *busy;
    ngx_http_chunked_t               *chunked;
    /* HTTP���������Ϻ�ִ�еĻص�������Ҳ����ngx_http_read_client_request_body�������ݵĵ�2������ */ 
    //POST���ݶ�ȡ��Ϻ���Ҫ���õ�HANDLER������Ҳ����ngx_http_upstream_init  
    ngx_http_client_body_handler_pt   post_handler; //��ngx_http_read_client_request_body�и�ֵ��ִ����ngx_http_do_read_client_request_body
} ngx_http_request_body_t;


typedef struct ngx_http_addr_conf_s  ngx_http_addr_conf_t;

//�ռ䴴���͸�ֵ��ngx_http_init_connection
//�ýṹ�洢�˷������˽��տͻ�������ʱ�������������ڵ�server{]������ctx  server_name��������Ϣ   ngx_http_request_t->http_connection��ngx_connection_t->data������ָ��ÿռ�
typedef struct {  //��ȡ�����Ӧ��server����Ϣ��server_name��Ϣ���Բο�ngx_http_set_virtual_server��Ϊ����
    //addr_conf = ngx_http_port_t->addrs[].conf 
    ngx_http_addr_conf_t             *addr_conf; //��ֵ��ngx_http_init_connection�����ݱ���IP��ַ��ȡ����Ӧ��addr_conf_t  ������洢serv loc��������Ϣ�Լ�server_name������Ϣ
    
    //�ͻ������ӷ����accept�ɹ��󣬸�accept��Ӧ��listen���ڵ�server{}���ÿ����ڵ�ctx������//��ֵ��ngx_http_init_connection��hc->conf_ctx = hc->addr_conf->default_server->ctx;
    ngx_http_conf_ctx_t              *conf_ctx;  //��server{}��������Ļ�ȡ���Բο�ngx_http_wait_request_handler
#if (NGX_HTTP_SSL && defined SSL_CTRL_SET_TLSEXT_HOSTNAME)  
    ngx_str_t                        *ssl_servername;
#if (NGX_PCRE)
    ngx_http_regex_t                 *ssl_servername_regex;
#endif
#endif

    ngx_buf_t                       **busy;
    ngx_int_t                         nbusy;

    ngx_buf_t                       **free;
    ngx_int_t                         nfree;

#if (NGX_HTTP_SSL)
    unsigned                          ssl:1;
#endif
    unsigned                          proxy_protocol:1; //listen���������Ƿ�Я���ò���
} ngx_http_connection_t;


typedef void (*ngx_http_cleanup_pt)(void *data);

typedef struct ngx_http_cleanup_s  ngx_http_cleanup_t;
/*
 ngx_pool_cleanup_t��ngx_http_cleanup_pt�ǲ�ͬ�ģ�ngx_pool_cleanup_t�������õ��ڴ������ʱ�Żᱻ������������Դ������ʱ�ͷ���
 Դ������ʹ�õ��ڴ�ض�������ngx_http_cleanup_pt����ngx_http_request_t�ṹ���ͷ�ʱ���������ͷ���Դ�ġ�


 �����Ҫ�������ͷ�ʱִ��һЩ�ص�������������Ҫʵ��һ��ngx_http_cleanup_pt��������Ȼ��HTTP��ܻ����Ѻõ��ṩ��һ������
 ����ngx_http_cleanup_add�����������������ngx_http_cleanup_t������
*/
//�κ�һ�������ngx_http_request_t�ṹ���ж���һ��ngx_http_cleanup_t���͵ĳ�Աcleanup
struct ngx_http_cleanup_s {
    ngx_http_cleanup_pt               handler; //��HTTPģ���ṩ��������Դ�Ļص�����  ngx_http_free_request ngx_http_terminate_request�����ͷſռ�
    void                             *data; //ϣ���������handler�������ݵĲ���
    //һ��������ܻ��ж��ngx_http_cleanup_t����������Щ�����������ͨ��nextָ�����ӳɵ������
    ngx_http_cleanup_t               *next;
};

//Nginx�����������������쳣����ʱ���������ngx_http_post_subrequest_pt�ص�����
typedef ngx_int_t (*ngx_http_post_subrequest_pt)(ngx_http_request_t *r,
    void *data, ngx_int_t rc);

/*
������ngx_http_post_subrequest_t�ṹ��ʱ�����԰��������ݸ��������dataָ�룬ngx_http_post_subrequest_pt�ص�����ִ��ʱ��
data��������ngx_http_po st_subrequest_t�ṹ���е�data��Աָ�롣
    ngx_http_post_subrequest_pt�ص������е�rc�������������ڽ���ʱ��״̬������ȡֵ����ִ��ngx_http_finalize_request������
��ʱ���ݵ�rc���������ڱ�����˵������������ʹ�÷������ģ���������HTTP������������rc��ʱ��HTTP��Ӧ�롣���磬���������
�£�rc����200����
*/
typedef struct {
    ngx_http_post_subrequest_pt       handler; //�ں���ngx_http_finalize_request��ִ��
    void                             *data;
} ngx_http_post_subrequest_t;


typedef struct ngx_http_postponed_request_s  ngx_http_postponed_request_t;
/*
������һ����������ʵ��������񸽣����ֻ��ϣ�����յ���������Ӧ����Nginx�н�����������ô����Ͳ���Ҫpostponeģ�飬����5.6����
���������������ɣ����ԭʼ������������������󣬲���ϣ�����������������Ӧ����ת�����ͻ��ˣ���Ȼ������ġ����Ρ����ǰ��մ���
�������˳����������Ӧ����ʱ��postponeģ������ˡ�����֮�ء���Nginx�е������������첽ִ�еģ��󴴽����������������ִ�У�����
ת�����ͻ��˵���Ӧ�ͻ�������ҡ���postponeģ���ǿ�ƵذѴ�ת������Ӧ�������һ�������з��ͣ�ֻ������ת���������������ŻῪʼ
ת����һ���������е���Ӧ���������һ���������ʵ�ֵġ�ÿ�������ngx_http_request_t�ṹ���ж���һ��postponed��Ա��


������������Կ��������ngx_http_postponed_request_t֮��ʹ��nextָ�����ӳ�һ����������ngx_http_postponed_request_t��
��out��Ա��ngx_chain_t�ṹ����ָ������������εġ���Ҫת�������ε���Ӧ���塣
    ÿ��ʹ��ngx_http_output_filter�������������ģ��Ҳʹ�ø÷���ת����Ӧ�����������ͻ��˷�����Ӧ����ʱ��������õ�
ngx_http_postpone_filter_module����ģ�鴦�����Ҫ���͵İ��塣���濴һ�¹��˰����ngx_http_postpone_filter���������Ķ����11�º���
��ͷ����δ��룬������ܻ������������
*/ //ngx_http_request_s�е�postponedΪ�ýṹ����
struct ngx_http_postponed_request_s {
    ngx_http_request_t               *request;
    ngx_chain_t                      *out;
    ngx_http_postponed_request_t     *next;
};


typedef struct ngx_http_posted_request_s  ngx_http_posted_request_t;
//ngx_http_request_t�е�posted_requestsʹ�øýṹ        //ngx_http_post_request�д���ngx_http_posted_request_t�ռ�
struct ngx_http_posted_request_s { //ͨ��posted_requests�ͰѸ����������Ե�����������ݽṹ��ʽ��֯����
    ngx_http_request_t               *request; //ָ��ǰ�������������ngx_http_request_t�ṹ��
    ngx_http_posted_request_t        *next; //ָ����һ�����������û�У���ΪNULL��ָ��
};

/*
#define NGX_HTTP_OK                        200
#define NGX_HTTP_CREATED                   201
#define NGX_HTTP_ACCEPTED                  202
#define NGX_HTTP_NO_CONTENT                204
#define NGX_HTTP_PARTIAL_CONTENT           206

#define NGX_HTTP_SPECIAL_RESPONSE          300
#define NGX_HTTP_MOVED_PERMANENTLY         301
#define NGX_HTTP_MOVED_TEMPORARILY         302
#define NGX_HTTP_SEE_OTHER                 303
#define NGX_HTTP_NOT_MODIFIED              304
#define NGX_HTTP_TEMPORARY_REDIRECT        307

#define NGX_HTTP_BAD_REQUEST               400
#define NGX_HTTP_UNAUTHORIZED              401
#define NGX_HTTP_FORBIDDEN                 403
#define NGX_HTTP_NOT_FOUND                 404
#define NGX_HTTP_NOT_ALLOWED               405
#define NGX_HTTP_REQUEST_TIME_OUT          408
#define NGX_HTTP_CONFLICT                  409
#define NGX_HTTP_LENGTH_REQUIRED           411
#define NGX_HTTP_PRECONDITION_FAILED       412
#define NGX_HTTP_REQUEST_ENTITY_TOO_LARGE  413
#define NGX_HTTP_REQUEST_URI_TOO_LARGE     414
#define NGX_HTTP_UNSUPPORTED_MEDIA_TYPE    415
#define NGX_HTTP_RANGE_NOT_SATISFIABLE     416

 The special code to close connection without any response 
#define NGX_HTTP_CLOSE                     444
#define NGX_HTTP_NGINX_CODES               494
#define NGX_HTTP_REQUEST_HEADER_TOO_LARGE  494
#define NGX_HTTPS_CERT_ERROR               495
#define NGX_HTTPS_NO_CERT                  496

#define NGX_HTTP_TO_HTTPS                  497
#define NGX_HTTP_CLIENT_CLOSED_REQUEST     499


#define NGX_HTTP_INTERNAL_SERVER_ERROR     500
#define NGX_HTTP_NOT_IMPLEMENTED           501
#define NGX_HTTP_BAD_GATEWAY               502
#define NGX_HTTP_SERVICE_UNAVAILABLE       503
#define NGX_HTTP_GATEWAY_TIME_OUT          504
#define NGX_HTTP_INSUFFICIENT_STORAGE      507

ע�⡡���Ϸ���ֵ����RFC2616�淶�ж���ķ������⣬����Nginx�������HTTP�����롣���磬NGX_HTTP_CLOSE��������Ҫ��HTTP���ֱ�ӹر��û����ӵġ�

�ڴ������г��˷���HTTP��Ӧ���⣬�����Է���Nginxȫ�ֶ���ļ��������룬������
#define  NGX_OK          0
#define  NGX_ERROR      -1
#define  NGX_AGAIN      -2
#define  NGX_BUSY       -3
#define  NGX_DONE       -4
#define  NGX_DECLINED   -5
#define  NGX_ABORT      -6

*/
//�������ֵ������HTTP����Ӧ���ķ����룬���а�����HTTP����Ѿ���src/http/ngx_http_request.h�ļ��ж���õĺ�
//��HTTPģ��ʵ�ֵ�handler����������������ھ�������д�Լ��Ĳ���ģ��ʱ������ngx_http_mytest_handler����ʵ�ֹ�

/*
��7��HTTP�׶�(NGX_HTTP_POST_READ_PHASE��NGX_HTTP_SERVER_REWRITE_PHASE��NGX_HTTP_REWRITE_PHASE��NGX_HTTP_PREACCESS_PHASE��
NGX_HTTP_ACCESS_PHASE��NGX_HTTP_CONTENT_PHASE��NGX_HTTP_LOG_PHASE)�������κ�һ��HTTPģ��ʵ���Լ���ngx_http_handler_pt��
��������������뵽��7���׶���ȥ�ġ��ڵ���HTTPģ���postconfiguration��������7���׶�����Ӵ�����ǰ����Ҫ�Ƚ�phases��
������7���׶����handlers��̬�����ʼ����ngx_array_t������Ҫִ��ngx_array_init������ʼ����������һ�����У�ͨ����
��ngx_http_init_phases��������ʼ����7����̬���顣

    ͨ����������HTTPģ���postconfiguration������HTTPģ���������һ�����н��Լ���ngx_http_handler_pt��������ӵ�����7��HTTP�׶��С�
�����ھ����ÿ���׶ξͿ���ִ�е����ǵ�handler�ص�

NGX_HTTP_CONTENT_PHASE�׶��������׶ζ�����ͬ���ǣ�����HTTPģ���ṩ�����ֽ���ý׶εķ�ʽ����һ��������10���׶�һ����
ͨ����ȫ�ֵ�ngx_http_core_main_conf_t�ṹ���phases���������ngx_http_handler_pt��������ʵ�֣����ڶ����Ǳ��׶ζ��еģ���ϣ�����������
ngx_http_handler_pt�������õ�location��ص�ngx_http_core_loc_conf_t�ṹ���handlerָ���У������ǵ�3����mytest���ӵ��÷���
*/ //CONTENT_PHASE�׶εĴ���ص�����ngx_http_handler_pt�Ƚ����⣬��ngx_http_core_content_phase 
typedef ngx_int_t (*ngx_http_handler_pt)(ngx_http_request_t *r); //����r��������ЩNginx�Ѿ���������û�������Ϣ
typedef void (*ngx_http_event_handler_pt)(ngx_http_request_t *r);

/*
POST / HTTP/1.1
Host: www.baidu.com
User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.7.6)
Gecko/20050225 Firefox/1.0.1
Content-Type: application/x-www-form-urlencoded
Content-Length: 40
Connection: Keep-Alive
name=Professional%20Ajax&publisher=Wiley

*/
/*
�����������Ϣ���緽����URI��Э��汾�ź�ͷ���ȣ��������ڴ����ngx_http_request_t���Ͳ���r��ȡ�á�
ngx_http_request_t�ṹ������ݺܶ࣬���ڲ���̽��ngx_http_request_t�����г�Ա�����壨ngx_http_request_t�ṹ���е�����Աֻ��HTTP��ܲŸ���Ȥ��
��11.3.1�ڻ����ϸ��˵������
*/
/*
ngx_http_core_main_conf_t->variabels�����Ա�Ľṹʽngx_http_variable_s�� ngx_http_request_s->variabels�����Ա�ṹ��
ngx_variable_value_t�������ṹ�Ĺ�ϵ�����У�һ����ν������һ����ν����ֵ
*/

/*
����ÿ��ngx_http_request_t������˵��ֻ�ܷ���һ�����η�������������һ���ͻ���������˵����������������������κ�һ��������
���Է���һ�����η���������Щ������Ľ����������Ϳ���ʹ���Կͻ��˵��������ӵ�ҵ��
*/
//ngx_http_parse_request_line���������У� ngx_http_process_request_headers����ͷ����(����ͷ��)
struct ngx_http_request_s { //�����յ��ͻ����������ݺ󣬵���ngx_http_create_request�д�������ֵ
    uint32_t                          signature;         /* "HTTP" */ 

    //�ڽ��յ��ͻ������ݺ󣬻ᴴ��һ��ngx_http_request_s����connection��Աָ���Ӧ��accept�ɹ����ȡ����������Ϣngx_connection_t����ngx_http_create_request
    ngx_connection_t                 *connection; //��������Ӧ�Ŀͻ�������

    /*
    ctx��ngx_http_conf_ctxt�ṹ��3�������Ա�ǳ����ƣ����Ƕ�
    ��ʾָ��void��ָ������顣HTTP��ܾ�����ctx�����б�������HTTPģ�������Ľṹ���ָ���,����ģ������������Ŀռ���
    ngx_http_create_request�д�������ȡ�����÷ֱ���ngx_http_get_module_ctx��ngx_http_set_ctx��Ϊÿ�����󴴽�ngx_http_request_s��ʱ��
    ����Ϊ�������ctx[]Ϊ���е�ģ�鴴��һ��ָ�룬Ҳ����ÿ��ģ����ngx_http_request_s����һ��ctx
    */ //��Ӧ�������ngx_http_filter_finalize_request���ctxָ��Ŀռ�ȫ����0
    void                            **ctx; //ָ��������HTTPģ��������Ľṹ���ָ������,ʵ���Ϸ��͸��ͻ��˵�Ӧ����ɺ󣬻��ctxȫ����0
    /*
    ���ͻ��˽������Ӻ󣬲������������ݹ�������ngx_http_create_request�д�ngx_http_connection_t->conf_ctx��ȡ������ֵ��Ҳ���Ǹ��ݿͻ�������
    ��������IP:port����Ӧ��Ĭ��server{}�������ģ�������������:ip:port��ͬ�����ڲ�ͬ��server{}���У���ô�п��ܿͻ������������ʱ��Я����host
    ͷ�����server_name����Ĭ�ϵ�server{}�У����������server{}�У�������Ҫͨ��ngx_http_set_virtual_server���»�ȡserver{}��location{}����������
    ����:
        server {  #1
            listen 1.1.1.1:80;
            server_name aaa
        }

        server {   #2
            listen 1.1.1.1:80;
            server_name bbb
        }
        ���������ngx_http_init_connection�а�ngx_http_connection_t->conf_ctxָ��ngx_http_addr_conf_s->default_server,Ҳ����ָ��#1,Ȼ��
        ngx_http_create_request�а�main_conf srv_conf  loc_conf ָ��#1,
        ����������е�ͷ����host:bbb����ô��Ҫ���»�ȡ��Ӧ��server{} #2,��ngx_http_set_virtual_server
*/ //ngx_http_create_request��ngx_http_set_virtual_server �Ѿ�rewrite������(����ngx_http_core_find_location 
//ngx_http_core_post_rewrite_phase ngx_http_internal_redirect ngx_http_internal_redirect ������ngx_http_subrequest)�����ܶ����Ǹ�ֵ
    void                            **main_conf; //ָ�������Ӧ�Ĵ��main�������ýṹ���ָ������
    void                            **srv_conf; //ָ�������Ӧ�Ĵ��srv�������ýṹ���ָ������   ��ֵ��ngx_http_set_virtual_server
    void                            **loc_conf; //ָ�������Ӧ�Ĵ��loc�������ýṹ���ָ������   ��ֵ��ngx_http_set_virtual_server

    /*
     �ڽ�����HTTPͷ������һ����ҵ���ϴ���HTTP����ʱ��HTTP����ṩ�Ĵ�������ngx_http_process_request��������÷����޷�һ�δ�
     ����������ȫ��ҵ���ڹ黹����Ȩ��epoll�¼�ģ��󣬸������ٴα��ص�ʱ����ͨ��ngx_http_request_handler���������������
     �����ж��ڿɶ��¼��Ĵ�����ǵ���read_event_handler��������Ҳ����˵��HTTPģ��ϣ���ڵײ㴦������Ķ��¼�ʱ������ʵ��read_event_handler����

     //�ڶ�ȡ�ͻ������İ���ʱ����ֵΪngx_http_read_client_request_body_handler
     �����ͻ��˵İ���ʱ����ֵΪngx_http_discarded_request_body_handler
     */ //ע��ngx_http_upstream_t��ngx_http_request_t���иó�Ա �ֱ���ngx_http_request_handler��ngx_http_upstream_handler��ִ��
    ngx_http_event_handler_pt         read_event_handler; //ngx_http_request_handler������ִ��

    /* ��read_event_handler�ص��������ƣ����ngx_http_request_handler�����жϵ�ǰ�¼��ǿ�д�¼��������write_event_handler�������� */
    /*�����к�����ͷ��������ɺ󣬻���ngx_http_handler�и�ֵΪngx_http_core_run_phases
       ��������Ӧ��ʱ�����һ��û�з����꣬������Ϊngx_http_writer
     */ //ע��ngx_http_upstream_t��ngx_http_request_t���иó�Ա �ֱ���ngx_http_request_handler��ngx_http_upstream_handler��ִ��
    ngx_http_event_handler_pt         write_event_handler;//���������¼����Ļص�����

#if (NGX_HTTP_CACHE)
    ngx_http_cache_t                 *cache;
#endif

    /* 
    ���û��ʹ��upstream���ƣ���ôngx_http_request_t�е�upstream��Ա��NULL��ָ��,��ngx_http_upstream_create�д����ռ�
    */
    ngx_http_upstream_t              *upstream; //upstream�����õ��Ľṹ��
    ngx_array_t                      *upstream_states; //�����ռ�͸�ֵ��ngx_http_upstream_init_request
                                         /* of ngx_http_upstream_state_t */

    /*
    ��ʾ���������ڴ�أ���ngx_http_free_request���������١�����ngx_connection-t�е��ڴ�����岻ͬ���������ͷ�ʱ��TCP���ӿ��ܲ�
    û�йرգ���ʱ������ڴ�ػ����٣���ngx_connection_t���ڴ�ز���������
     */
    ngx_pool_t                       *pool;
    //���У�header_inָ��Nginx�յ���δ��������HTTPͷ���������ݲ���ע�����ڵ�11���п��Կ�����header_in���ǽ���HTTPͷ���Ļ��������� header_in��������У��Headers_in���ͷ����
    //�����к�����ͷ�����ݶ��ڸ�buffer��
    ngx_buf_t                        *header_in;//���ڽ���HTTP�������ݵĻ���������Ҫ���ڽ���HTTPͷ������ָ��ָ��ngx_connection_t->buffer
    //���͵�headers_in��洢�Ѿ���������HTTPͷ�����������ngx_http_headers_in_t�ṹ���еĳ�Ա��
/*
 ngx_http_process_request_headers�����ڽ��ա�������HTTP�����ͷ���󣬻�ѽ������ÿһ��HTTPͷ�����뵽headers_in��headers�����У�ͬʱ�ṹ��headers_in�е�������Ա
 */ //�ο�ngx_http_headers_in��ͨ���������еĻص�hander���洢��������������name:value�е�value��headers_in����Ӧ��Ա�У���ngx_http_process_request_headers
    //ע��:����Ҫ�ѿͻ�������ͷ���͵���˵Ļ���������ͷ������������HTTP_��ر���������fastcgi����ngx_http_fastcgi_create_request
    ngx_http_headers_in_t             headers_in; //httpͷ���н���������ݶ��ɸó�Ա�洢  header_in��������У�headers_in���ͷ����
    //ֻҪָ��headers_out�еĳ�Ա���Ϳ����ڵ���ngx_http_send_headerʱ��ȷ�ذ�HTTPͷ������
    //HTTPģ������Ҫ���͵�HTTP��Ӧ��Ϣ�ŵ�headers_out�У�����HTTP��ܽ�headers_out�еĳ�Ա���л�ΪHTTP��Ӧ�����͸��û�
    ngx_http_headers_out_t            headers_out; 
    //�����upstream��ֵ����Դ�Ǻ�˷��������е�ͷ�����п������ο�ngx_http_upstream_headers_in�е�copy_handler

/*
����������İ���󣬿�����r->request_body->temp_file->file�л�ȡ��ʱ�ļ����ٶ���r->request_body_in_file_only��־λ��Ϊ1���Ǿ�һ������
�����������ȡ�����塣����file��һ��ngx_file_t���͡�������ǿ��Դ�
r->request_body->temp_file->file.name�л�ȡNginx���յ���������������ļ������ƣ�����·������
*/ //��ngx_http_read_client_request_body�з���洢�ռ� ��ȡ�Ŀͻ��˰���洢��r->request_body->bufs�������ʱ�ļ�r->request_body->temp_file�� ngx_http_read_client_request_body
//��ȡ�ͻ����弴ʹ�Ǵ�����ʱ�ļ��У������а����ȡ��Ϻ�(��ngx_http_do_read_client_request_body)�����ǻ���r->request_body->bufsָ���ļ��е����ƫ���ڴ��ַ
    ngx_http_request_body_t          *request_body; //����HTTP�����а�������ݽṹ��ΪNULL��ʾ��û�з���ռ�
    //min(lingering_time,lingering_timeout)���ʱ���ڿ��Լ�����ȡ���ݣ�����ͻ����з������ݹ�������ngx_http_set_lingering_close
    time_t                            lingering_time; //�ӳٹر����ӵ�ʱ��
    //ngx_http_request_t�ṹ������������Ա��ʾ�������Ŀ�ʼ����ʱ�䣺start sec��Ա��start msec��Ա
    /*
     ��ǰ�����ʼ��ʱ��ʱ�䡣start sec�Ǹ�������ʱ��1970��1��1���賿0��0��0�뵽��ǰʱ������������������������������ʱ��
     �������������ʱ�䣻�������������û����������������ڽ�����TCP���Ӻ󣬵�һ�ν��յ��ɶ��¼�ʱ��ʱ��
     */
    time_t                            start_sec;
    ngx_msec_t                        start_msec;//��start_sec���ʹ�ã���ʾ�����start_set��ĺ���ƫ����



//����9����Ա����ngx_http_proces s_request_line�����ڽ��ա�����HTTP������ʱ����������Ϣ
/*
ע�⡡Nginx�ж��ڴ�Ŀ����൱�ϸ�Ϊ�˱��ⲻ��Ҫ���ڴ濪���������Ҫ�õ��ĳ�Ա���������·����ڴ��洢�ģ�����ֱ��ָ���û������е���Ӧ��ַ��
���磬method_name.data��request_start������ָ��ʵ��ָ��Ķ���ͬһ����ַ�����ң���Ϊ�����Ǽ򵥵��ڴ�ָ�룬����ָ���ַ�����ָ�룬���ԣ��ڴ󲿷�����£������ܽ���Щu_char*ָ�뵱���ַ���ʹ�á�
*/ //NGX_HTTP_GET | NGX_HTTP_HEAD��,ΪNGX_HTTP_HEAD��ʾֻ��Ҫ����HTTPͷ���ֶ�
    ngx_uint_t                        method; //��Ӧ�ͻ��������������е����󷽷�GET��POS�ȣ�ȡֵ��NGX_HTTP_GET,Ҳ�����������method_name�����ַ����Ƚ�
/*
http_protocolָ���û�������HTTP����ʼ��ַ��
http_version��Nginx��������Э��汾������ȡֵ��Χ���£�
#define NGX_HTTP_VERSION_9                 9
#define NGX_HTTP_VERSION_10                1000
#define NGX_HTTP_VERSION_11                1001
����ʹ��http_version����HTTP��Э��汾��
���ʹ��request_start��request_end���Ի�ȡԭʼ���û������С�
*/
    ngx_uint_t                        http_version;//http_version��Nginx��������Э��汾������ȡֵ��Χ���£�

    ngx_str_t                         request_line; //����������
//ngx_str_t���͵�uri��Աָ���û������е�URI��ͬ��u_char*���͵�uri_start��uri_endҲ��request_start��method_end���÷����ƣ�Ψһ��
//ͬ���ǣ�method_endָ�򷽷��������һ���ַ�����uri_endָ��URI���������һ����ַ��Ҳ�������һ���ַ�����һ���ַ���ַ��HTTP��ܵ���Ϊ����
//���Ǵ󲿷�u_char*����ָ��ԡ�xxx_start���͡�xxx_end���������÷���
    //http://10.135.10.167/mytest�е�/mytest  http://10.135.10.167/mytest?abc?ttt�е�/mytest  
    //ͬʱ"GET /mytest?abc?ttt HTTP/1.1"�е�mytest��uri�е�һ��    
    ngx_str_t                         uri; 
    //argָ���û������е�URL������  http://10.135.10.167/mytest?abc?ttt�е�abc?ttt   
    //ͬʱ"GET /mytest?abc?ttt HTTP/1.1"�е�mytest?abc?ttt��uri�е�һ��    
    ngx_str_t                         args;
    /*
    ngx_str_t���͵�extern��Աָ���û�������ļ���չ�������磬�ڷ��ʡ�GET /a.txt HTTP/1.1��ʱ��extern��ֵ��{len = 3, data = "txt"}��
    ���ڷ��ʡ�GET /a HTTP/1.1��ʱ��extern��ֵΪ�գ�Ҳ����{len = 0, data = 0x0}��
    uri_extָ��ָ��ĵ�ַ��extern.data��ͬ��
    */
    ngx_str_t                         exten; //http://10.135.10.167/mytest/ac.txt�е�txt
/*
url�����г���+���ո�=��%��&��#���ַ��Ľ���취 
url��������+���ո�/��?��%��#��&��=��������ŵ�ʱ�򣬿����ڷ��������޷������ȷ�Ĳ���ֵ������Ǻã�
����취
����Щ�ַ�ת���ɷ���������ʶ����ַ�����Ӧ��ϵ���£�
URL�ַ�ת��

�������ַ�����ɣ�����ȫ�ǵġ�

+    URL ��+�ű�ʾ�ո�                      %2B   
�ո� URL�еĿո������+�Ż��߱���           %20 
/   �ָ�Ŀ¼����Ŀ¼                        %2F     
?    �ָ�ʵ�ʵ�URL�Ͳ���                    %3F     
%    ָ�������ַ�                           %25     
#    ��ʾ��ǩ                               %23     
&    URL ��ָ���Ĳ�����ķָ���             %26     
=    URL ��ָ��������ֵ                     %3D
*/
//unparsed_uri��ʾû�н���URL�����ԭʼ�������磬��uriΪ��/a b��ʱ��unparsed_uri�ǡ�/a%20b�����ո��ַ�����������%20����
    ngx_str_t                         unparsed_uri;//�ο�:ΪʲôҪ��URI���б���:
    ngx_str_t                         method_name;//��method   GET  POST��
    ngx_str_t                         http_protocol;//GET /sample.jsp HTTP/1.1  �е�HTTP/1.1







/* ��ngx_http_header_filter�����޷�һ���Է���HTTPͷ��ʱ��������������������ͬʱ����:�����out��Ա�н��ᱣ��ʣ�����Ӧͷ��,��ngx_http_header_filter */    
/* ��ʾ��Ҫ���͸��ͻ��˵�HTTP��Ӧ��out�б�������headers_out�����л���ı�ʾHTTPͷ����TCP�����ڵ���ngx_http_output_filter������
out�л��ᱣ������͵�HTTP���壬����ʵ���첽����HTTP��Ӧ�Ĺؼ� */
    ngx_chain_t                      *out;
/* ��ǰ����ȿ������û�����������Ҳ�������������������󣬶�main���ʶһϵ����ص���������
���ԭʼ��������һ���ͨ��main�͵�ǰ����ĵ�ַ�Ƿ�������жϵ�ǰ�����Ƿ�Ϊ�û�������ԭʼ���� */
    //main��Աʼ��ָ��һϵ������Ե��ϵ�������е�Ψһ���Ǹ�ԭʼ����,��ʼ��ֵ��ngx_http_create_request
    ngx_http_request_t               *main;//���Բο�ngx_http_core_access_phase
    ngx_http_request_t               *parent;//��ǰ����ĸ�����ע�⣬������δ����ԭʼ����

/*
������һ����������ʵ��������񸽣����ֻ��ϣ�����յ���������Ӧ����Nginx�н�����������ô����Ͳ���Ҫpostponeģ�飬����5.6����
���������������ɣ����ԭʼ������������������󣬲���ϣ�����������������Ӧ����ת�����ͻ��ˣ���Ȼ������ġ����Ρ����ǰ��մ���
�������˳����������Ӧ����ʱ��postponeģ������ˡ�����֮�ء���Nginx�е������������첽ִ�еģ��󴴽����������������ִ�У�����
ת�����ͻ��˵���Ӧ�ͻ�������ҡ���postponeģ���ǿ�ƵذѴ�ת������Ӧ�������һ�������з��ͣ�ֻ������ת���������������ŻῪʼ
ת����һ���������е���Ӧ���������һ���������ʵ�ֵġ�ÿ�������ngx_http_request_t�ṹ���ж���һ��postponed��Ա��

������������Կ��������ngx_http_postponed_request_t֮��ʹ��nextָ�����ӳ�һ����������ngx_http_postponed_request_t��
��out��Ա��ngx_chain_t�ṹ����ָ������������εġ���Ҫת�������ε���Ӧ���塣
    ÿ��ʹ��ngx_http_output_filter�������������ģ��Ҳʹ�ø÷���ת����Ӧ�����������ͻ��˷�����Ӧ����ʱ��������õ�
ngx_http_postpone_filter_module����ģ�鴦�����Ҫ���͵İ��塣���濴һ�¹��˰����ngx_http_postpone_filter���������Ķ����11�º���
��ͷ����δ��룬������ܻ������������
*/
    ngx_http_postponed_request_t     *postponed; //��subrequest��������صĹ���
    ngx_http_post_subrequest_t       *post_subrequest;
/* ���е���������ͨ��posted_requests��������������������ģ�ִ��post������ʱ���õ�
ngx_http_run_posted_requests��������ͨ�������õ�������ִ��������� */ //ngx_http_post_request�д���ngx_http_posted_request_t�ռ�
    ngx_http_posted_request_t        *posted_requests; //ͨ��posted_requests�ͰѸ����������Ե�����������ݽṹ��ʽ��֯����

/*
ȫ�ֵ�ngx_http_phase_engine_t�ṹ���ж�����һ��ngx_http_phase_handler_t�ص�������ɵ����飬��phase_handler��Ա������������ʹ�ã�
��ʾ�����´�Ӧ��ִ����phase_handler��Ϊ���ָ���������еĻص�������HTTP������������ַ�ʽ�Ѹ���HTTP���鼯���������������
*///phase_handlerʵ�����Ǹý׶εĴ�����������ngx_http_phase_engine_t->handlers�����е�λ��
    ngx_int_t                         phase_handler; 
    //��ʾNGX HTTP CONTENT PHASE�׶��ṩ��HTTPģ�鴦�������һ�ַ�ʽ��content handlerָ��HTTPģ��ʵ�ֵ���������,��ngx_http_core_content_phase��ִ��
    ngx_http_handler_pt               content_handler; ////��ngx_http_update_location_config�и�ֵ��r->content_handler = clcf->handler;
/*
    ��NGX_HTTP_ACCESS_PHASE�׶���Ҫ�ж������Ƿ���з���Ȩ��ʱ��ͨ��access_code������HTTPģ���handler�ص������ķ���ֵ�����access_codeΪ0��
���ʾ����߱�����Ȩ�ޣ���֮��˵�����󲻾߱�����Ȩ��

    NGXHTTPPREACCESSPHASE��NGX_HTTP_ACCESS_PHASE��NGX HTTPPOST_ACCESS_PHASE���ܺ���⣬������Ȩ�޼���ǰ�ڡ����ڡ����ڹ�����
���к��ڹ����ǹ̶��ģ��ж�ǰ�����Ȩ�޼��Ľ����״̬�������ֶ�r->access_code�ڣ��������ǰ����û�з���Ȩ�ޣ���ôֱ�ӷ���״
̬403������������׶�Ҳ�޷�ȥ���ض���Ļص�������
*/
    ngx_uint_t                        access_code; //��ֵ��ngx_http_core_access_phase
    /*
    ngx_http_core_main_conf_t->variables�����Ա�Ľṹʽngx_http_variable_s�� ngx_http_request_s->variables�����Ա�ṹ��ngx_variable_value_t,
    �������ṹ�Ĺ�ϵ�����У�һ����ν������һ����ν����ֵ

    r->variables���������cmcf->variables��һһ��Ӧ�ģ��γ�var_ name��var_value�ԣ����������������ͬһ���±�λ��Ԫ�ظպþ���
�໥��Ӧ�ı������ͱ���ֵ����������ʹ��ĳ������ʱ�ܻ���ͨ������ngx_http_get_variable_index������ڱ������������index�±꣬Ҳ���Ǳ�
�������index�ֶ�ֵ��Ȼ���������index�±����ȥ����ֵ������ȡ��Ӧ��ֵ
    */ //����Ľڵ�����ngx_http_create_request����ngx_http_core_main_conf_t->variablesһһ��Ӧ
    //����ngx_http_script_var_code_t->index��ʾNginx����$file��ngx_http_core_main_conf_t->variables�����ڵ��±꣬��Ӧÿ������ı���ֵ�洢�ռ��Ϊr->variables[code->index],�ο�ngx_http_script_set_var_code
    ngx_http_variable_value_t        *variables; //ע���ngx_http_core_main_conf_t->variables������

#if (NGX_PCRE)
    /*  
     ����������ʽ���re.name= ^(/download/.*)/media/(.*)/tt/(.*)$��  s=/download/aa/media/bdb/tt/ad,�����ǻ�ƥ�䣬ͬʱƥ���
     ��������3�����򷵻�ֵΪ3+1=4,�����ƥ���򷵻�-1

     ����*2����Ϊ��ȡǰ�������е�3��������Ӧ��ֵ��Ҫ�ɶ�ʹ��r->captures���ο�ngx_http_script_copy_capture_code��
     */
    ngx_uint_t                        ncaptures; //��ֵ��ngx_http_regex_exec   //����$n*2
    int                              *captures; //ÿ����ͬ���������֮��Ľ������������$1,$2��
    u_char                           *captures_data; //����������ʽƥ���ԭ�ַ���������http://10.135.2.1/download/aaa/media/bbb.com�е�/download/aaa/media/bbb.com
#endif

/* limit_rate��Ա��ʾ������Ӧ��������ʣ���������0ʱ����ʾ��Ҫ���١�limit rate��ʾÿ����Է��͵��ֽ���������������־���Ҫ���٣�
Ȼ����������������������ڷ�����limit_rate_after�ֽڵ���Ӧ�������Ч������С��Ӧ�����Ż���ƣ� */
//ʵ�����ͨ��ngx_writev_chain�������ݵ�ʱ�򣬻�������һ��
    size_t                            limit_rate; //���ٵ���ؼ��㷽���ο�ngx_http_write_filter
    size_t                            limit_rate_after;

    /* used to learn the Apache compatible response length without a header */
    size_t                            header_size; //����ͷ��������֮�ͣ����Բο�ngx_http_header_filter

    off_t                             request_length; //HTTP�����ȫ�����ȣ�����HTTP����

    ngx_uint_t                        err_status;

    //�����ӽ����ɹ��󣬵��յ��ͻ��˵ĵ�һ�������ʱ���ͨ��ngx_http_wait_request_handler->ngx_http_create_request����ngx_http_request_t
    //ͬʱ��r->http_connectionָ��accept�ͻ������ӳɹ�ʱ�򴴽���ngx_http_connection_t���������д洢server{}������ctx��server_name����Ϣ
    //��ngx_http_request_t��һֱ��Ч�����ǹر����ӡ���˸ú���ֻ�����һ�Σ�Ҳ���ǵ�һ���ͻ��������Ĺ�����ʱ�򴴽���һֱ���������ӹر�
    //�ýṹ�洢�˷������˽��տͻ�������ʱ�������������ڵ�server{]������ctx  server_name��������Ϣ
    ngx_http_connection_t            *http_connection; //�洢ngx_connection_t->dataָ���ngx_http_connection_t����ngx_http_create_request
#if (NGX_HTTP_SPDY)
    ngx_http_spdy_stream_t           *spdy_stream;
#endif

    ngx_http_log_handler_pt           log_handler;
    //������������������ĳЩ��Դ������Ҫ���������ʱ�ͷţ���ô����Ҫ�ڰѶ�����ͷ���Դ������ӵ�cleanup��Ա��
    /* 
    ���û����Ҫ�������Դ����cleanupΪ��ָ�룬����HTTPģ�������cleanup���Ե��������ʽ�����Ƶ����ngx_http_cleanup_t�ṹ�壬
    �������������ʱ�ͷ���Դ */
    ngx_http_cleanup_t               *cleanup;

    unsigned                          subrequests:8;

/*
���Ķ�HTTP�������ģ��(ngx_http_proxy_module)Դ����ʱ���ᷢ������û�е���r->main->count++������proxyģ������������upstream���Ƶģ�
ngx_http_read_client_request_body(r��ngx_http_upstream_init);�����ʾ��ȡ���û������HTTP�����Ż����ngx_http_upstream_init����
����upstream���ƣ��μ�3.6.4�ڣ�������ngx_http_read_client_request_body�ĵ�һ����Ч�����r->maln->count++������HTTP�������ģ�鲻��
�ٴ����������ִ��r->main->count++��

������̿������ƺ���������Ϊʲô��ʱ��Ҫ�����ü�����1����ʱȴ����Ҫ�أ���Ϊngx_http_read- client_request_body��ȡ���������
һ���첽��������Ҫepoll��ε��ȷ�����ɵĿɳ���Ϊ�첽��������ngx_http_upstream_init��������upstream����Ҳ��һ���첽��������ˣ�
����������˵��ÿִ��һ���첽����Ӧ�ð����ü�����1�����첽��������ʱӦ�õ���ngx_http_finalize_request���������ü�����1�����⣬
ngx_http_read_client_request_body�������Ǽӹ����ü����ģ���ngx_http_upstream_init������ȴû�мӹ����ü���������Nginx�������޸�
������⣩����HTTP�������ģ���У�����ngx_http_proxy_handler�������á�ngx_http_read- client_request_body(r��ngx_http_upstream_init);��
���ͬʱ�����������첽������ע�⣬���������ֻ����һ�����ü�����ִ����������ngx_http_proxy_handler��������ʱֻ����
ngx_http_finalize_request����һ�Σ�������ȷ�ġ�����mytestģ��Ҳһ�������Ҫ��֤�����ü��������Ӻͼ�������Խ��еġ�
*/
/*
��ʾ��ǰ��������ô��������磬��ʹ��subrequest����ʱ����������������ϵ���������Ŀ�᷵�ص�count�ϣ�ÿ����һ��������count����Ҫ��1��
�����κ�һ���������������µ�������ʱ����Ӧ��ԭʼ����mainָ��ָ������󣩵�countֵ��Ҫ��1�����磬�����ǽ���HTTP����ʱ��������Ҳ��
һ���첽���ã�����count��Ҳ��Ҫ��1�������ڽ�������ʱ���Ͳ�����count���ü���δ����ʱ��������
*/
    unsigned                          count:8; //Ӧ�ü���   ngx_http_close_request��-1
    /* ���AIO�������л��ڴ����������blocked��Ȼ�Ǵ���0�ģ���ʱngx_http_close_request�������ܽ������� */
    unsigned                          blocked:8; //������־λ��Ŀǰ����aioʹ��  Ϊ0����ʾû��HTTPģ�黹��Ҫ��������

    unsigned                          aio:1;  //��־λ��Ϊ1ʱ��ʾ��ǰ��������ʹ���첽�ļ�IO

    unsigned                          http_state:4;

    /* URI with "/." and on Win32 with "//" */
    unsigned                          complex_uri:1;

    /* URI with "%" */
    unsigned                          quoted_uri:1;

    /* URI with "+" */
    unsigned                          plus_in_uri:1;

    /* URI with " " */
    unsigned                          space_in_uri:1;

    unsigned                          invalid_header:1; //ͷ���н�������ȷ����ngx_http_parse_header_line

    unsigned                          add_uri_to_alias:1;
    unsigned                          valid_location:1; //ngx_http_handler����1
    unsigned                          valid_unparsed_uri:1;

    /*
    ��uri_changed����Ϊ0��Ҳ�ͱ�־˵URLû�б仯����ô����ngx_http_core_post_rewrite_phase�оͲ���ִ�������if��䣬Ҳ�Ͳ���
    �ٴ��ߵ�find config�Ĺ����ˣ����Ǽ����������ġ���Ȼ���������rewrite�ɹ����ǻ�������һ�εģ��൱��һ��ȫ�µ�����
     */ // ����rewrite   ^.*$ www.galaxywind.com last;�ͻ���ִ��rewrite       ngx_http_script_regex_start_code����1
    unsigned                          uri_changed:1; //��־λ��Ϊ1ʱ��ʾURL������rewrite��д  ֻҪ����rewrite xxx bbb sss;aaa����break����������1
    //��ʾʹ��rewrite��дURL�Ĵ�������ΪĿǰ�����Ը���10�Σ�����uri_changes��ʼ��Ϊ11����ÿ��дURL -�ξͰ�uri_changes��1��
    //һ��uri_changes����0�������û�����ʧ��
    unsigned                          uri_changes:4; //NGX_HTTP_MAX_URI_CHANGES + 1;

    unsigned                          request_body_in_single_buf:1;//client_body_in_single_buffer on | off;����
    //��1������Ҫ������ʱ�ļ���  ���request_body_no_bufferingΪ1��ʾ���û�����壬��ôrequest_body_in_file_onlyҲΪ0����Ϊ���û�����壬��ô�Ͳ���д����ʱ�ļ���
    /*ע��:���ÿ�ο��ٵ�client_body_buffer_size�ռ䶼�洢���˻�û�ж�ȡ�������İ��壬���ǻ��֮ǰ�����˵�buf�е����ݿ�������ʱ�ļ����ο�
        ngx_http_do_read_client_request_body -> ngx_http_request_body_filter��ngx_http_read_client_request_body -> ngx_http_request_body_filter
     */
    unsigned                          request_body_in_file_only:1; //"client_body_in_file_only on |clean"���� ��request_body_no_buffering�ǻ����
    unsigned                          request_body_in_persistent_file:1; //"client_body_in_file_only on"����
    unsigned                          request_body_in_clean_file:1;//"client_body_in_file_only clean"����
    unsigned                          request_body_file_group_access:1; //�Ƿ�����Ȩ�ޣ������һ��Ϊ0600
    unsigned                          request_body_file_log_level:3;
    //Ĭ����Ϊ0�ı�ʾ��Ҫ���棬���request_body_no_bufferingΪ1��ʾ���û�����壬��ôrequest_body_in_file_onlyҲΪ0����Ϊ���û�����壬��ô�Ͳ���д����ʱ�ļ���
    unsigned                          request_body_no_buffering:1; //�Ƿ񻺴�HTTP���壬�����������壬��request_body_in_file_only�ǻ���ģ���ngx_http_read_client_request_body

    /*
    upstream��3�ִ���������Ӧ����ķ�ʽ����HTTPģ����θ���
    upstreamʹ����һ�ַ�ʽ�������ε���Ӧ�����أ��������ngx_http_request_t�ṹ����
    subrequest_in_memory��־λΪ1ʱ�������õ�1�ַ�ʽ����upstream��ת����Ӧ����
    �����Σ���HTTPģ��ʵ�ֵ�input_filter����������壻��subrequest_in_memoryΪ0ʱ��
    upstream��ת����Ӧ���塣��ngx_http_upstream_conf t���ýṹ���е�buffering��־λΪ1
    ����������������ڴ�ʹ����ļ����ڻ������ε���Ӧ���壬����ζ�������ٸ��죻��buffering
    Ϊ0ʱ����ʹ�ù̶���С�Ļ�����������������ܵ�buffer����������ת����Ӧ���塣
    */
    unsigned                          subrequest_in_memory:1;
    unsigned                          waited:1;

#if (NGX_HTTP_CACHE)
    unsigned                          cached:1;
#endif

#if (NGX_HTTP_GZIP)
    unsigned                          gzip_tested:1;
    unsigned                          gzip_ok:1;
    unsigned                          gzip_vary:1;
#endif

    unsigned                          proxy:1;
    unsigned                          bypass_cache:1;
    unsigned                          no_cache:1;

    /*
     * instead of using the request context data in
     * ngx_http_limit_conn_module and ngx_http_limit_req_module
     * we use the single bits in the request structure
     */
    unsigned                          limit_conn_set:1;
    unsigned                          limit_req_set:1;

#if 0
    unsigned                          cacheable:1;
#endif

    unsigned                          pipeline:1;
    unsigned                          chunked:1;
    unsigned                          header_only:1; //��ʾ�Ƿ�ֻ���С�ͷ����û�а���  ngx_http_header_filter����1
    //��1.0���ϰ汾Ĭ���ǳ����ӣ�1.0���ϰ汾Ĭ����1�����������ͷ����û���������ӷ�ʽ����ngx_http_handler
    //��־λ��Ϊ1ʱ��ʾ��ǰ������keepalive����  1������   0������  ������ʱ��ͨ������ͷ����Keep-Alive:���ã��ο�ngx_http_headers_in_t
    unsigned                          keepalive:1;  //��ֵ��ngx_http_handler
//�ӳٹرձ�־λ��Ϊ1ʱ��ʾ��Ҫ�ӳٹرա����磬�ڽ�����HTTPͷ��ʱ������ְ�����ڣ��ñ�־λ����Ϊ1�����������հ���ʱ�����Ϊo
    unsigned                          lingering_close:1; 
    //���discard_bodyΪ1����֤������ִ�й���������ķ��������ڰ������ڱ������У���ngx_http_read_client_request_body
    unsigned                          discard_body:1;//��־ס��Ϊ1ʱ��ʾ���ڶ���HTTP�����еİ���
    unsigned                          reading_body:1; //��ǰ��廹û�ж��꣬��Ҫ������ȡ���壬��ngx_http_read_client_request_body

    /* ����һ�����У���phase_handler�����Ϊserver_rewrite_index������ζ������֮ǰִ�е���һ���׶Σ����϶�Ҫ���´�NGX_HTTP_SERVER_REWRITE_PHASE
�׶ο�ʼ�ٴ�ִ�У�����Nginx��������Է���rewrite�ض���Ļ�������ngx_http_handler */ //ngx_http_internal_redirect��1
//�ڲ��ض����Ǵ�NGX_HTTP_SERVER_REWRITE_PHASE������ִ��(ngx_http_internal_redirect)��������rewrite�Ǵ�NGX_HTTP_FIND_CONFIG_PHASE��ִ��(ngx_http_core_post_rewrite_phase)
    unsigned                          internal:1;//t��־λ��Ϊ1ʱ��ʾ����ĵ�ǰ״̬�������ڲ���ת����ֱ��
    unsigned                          error_page:1;
    unsigned                          filter_finalize:1;
    unsigned                          post_action:1;
    unsigned                          request_complete:1;
    unsigned                          request_output:1;
    //ΪIʱ��ʾ���͸��ͻ��˵�HTTP��Ӧͷ���Ѿ����͡��ڵ���ngx_http_send_header���������Ѿ��ɹ���������Ӧͷ���������̣�
    //�ñ�־λ�ͻ���Ϊ1��������ֹ�����ط���ͷ��
    unsigned                          header_sent:1;
    unsigned                          expect_tested:1;
    unsigned                          root_tested:1;
    unsigned                          done:1;
    unsigned                          logged:1;

    unsigned                          buffered:4;//��ʾ�������Ƿ��д��������ݵı�־λ

    unsigned                          main_filter_need_in_memory:1;
    unsigned                          filter_need_in_memory:1;
    unsigned                          filter_need_temporary:1;
    unsigned                          allow_ranges:1;  //֧�ֶϵ�����
    unsigned                          single_range:1;
    unsigned                          disable_not_modified:1;

#if (NGX_STAT_STUB)
    unsigned                          stat_reading:1;
    unsigned                          stat_writing:1;
#endif

    /* used to parse HTTP headers */ //״̬������HTTPʱʹ��state����ʾ��ǰ�Ľ���״̬
    ngx_uint_t                        state; //����״̬����ngx_http_parse_header_line
    //header_hashΪAccept-Language:zh-cn��Accept-Language�����ַ�����hash����Ľ��
    ngx_uint_t                        header_hash; //ͷ������һ���������ݼ���ngx_hash�Ľṹ���ο�ngx_http_parse_header_line
    //lowcase_indexΪAccept-Language:zh-cn��Accept-Language�ַ�����Ҳ����15���ֽ�
    ngx_uint_t                        lowcase_index; // �ο�ngx_http_parse_header_line
    //�洢Accept-Language:zh-cn�е�Accept-Language�ַ�����lowcase_header�������AAA_BBB:CCC,�������洢����_BBB
    u_char                            lowcase_header[NGX_HTTP_LC_HEADER_LEN]; //httpͷ�����ݣ�������Ӧ���л��������У��ο�ngx_http_parse_header_line

/*
����:Accept:image/gif.image/jpeg,** 
Accept��Ӧ��key��header_name_start header_name_end�ֱ�ָ�����Accept�ַ�����ͷ��β
image/gif.image/jpeg,** Ϊvalue���֣�header_start header_end�ֱ��Ӧvalue��ͷ��β�����Բο�mytest_upstream_process_header
*/
    //header_name_startָ��Accept-Language:zh-cn�е�A��
    u_char                           *header_name_start; //��������һ��httpͷ�����е�һ�е�name��ʼ�� //��ֵ��ngx_http_parse_header_line
    //header_name_startָ��Accept-Language:zh-cn�е�:��
    u_char                           *header_name_end; //��������һ��httpͷ�����е�һ�е�name��β�� //��ֵ��ngx_http_parse_header_line
    u_char                           *header_start;//header_startָ��Accept-Language:zh-cn�е�z�ַ���
    u_char                           *header_end;//header_endָ��Accept-Language:zh-cn�е�ĩβ���д�

    /*
     * a memory that can be reused after parsing a request line
     * via ngx_http_ephemeral_t
     */

//ngx_str_t���͵�uri��Աָ���û������е�URI��ͬ��u_char*���͵�uri_start��uri_endҲ��request_start��method_end���÷����ƣ�Ψһ��
//ͬ���ǣ�method_endָ�򷽷��������һ���ַ�����uri_endָ��URI���������һ����ַ��Ҳ�������һ���ַ�����һ���ַ���ַ��HTTP��ܵ���Ϊ����
//���Ǵ󲿷�u_char*����ָ��ԡ�xxx_start���͡�xxx_end���������÷���
    u_char                           *uri_start;
    u_char                           *uri_end;

/*
ngx_str_t���͵�extern��Աָ���û�������ļ���չ�������磬�ڷ��ʡ�GET /a.txt HTTP/1.1��ʱ��extern��ֵ��{len = 3, data = "txt"}��
���ڷ��ʡ�GET /a HTTP/1.1��ʱ��extern��ֵΪ�գ�Ҳ����{len = 0, data = 0x0}��
uri_extָ��ָ��ĵ�ַ��extern.data��ͬ��
*/ //GET /sample.jsp HTTP/1.1 ������ļ������.�ַ�����ָ���.�����jsp�ַ�������ʾ�ļ���չ��
    u_char                           *uri_ext;
    //"GET /aaaaaaaa?bbbb.txt HTTP/1.1"�е�bbb.txt�ַ���ͷλ�ô�
    u_char                           *args_start;//args_startָ��URL��������ʼ��ַ�����uri_endʹ��Ҳ���Ի��URL������

//request_start��method_end���÷�Ҳ�ܼ򵥣�����request_startָ���û�������׵�ַ��ͬʱҲ�Ƿ������ĵ�ַ��method_endָ�򷽷���
//�����һ���ַ���ע�⣬���������xxx_endָ�벻ͬ������ȡ������ʱ���Դ�request_start��ʼ��������ֱ����ַ��method_end��ͬΪֹ������ڴ�洢�ŷ�������
    u_char                           *request_start; //�����п�ʼ��
    u_char                           *request_end;  //�����н�β��
    u_char                           *method_end;  //GET  POST�ַ�����β��
    u_char                           *schema_start;
    u_char                           *schema_end;
    u_char                           *host_start;
    u_char                           *host_end;
    u_char                           *port_start;
    u_char                           *port_end;

    // HTTP/1.1ǰ���1����major�������1����minor
    unsigned                          http_minor:16;
    unsigned                          http_major:16;
};


typedef struct {
    ngx_http_posted_request_t         terminal_posted_request;
} ngx_http_ephemeral_t;


#define ngx_http_ephemeral(r)  (void *) (&r->uri_start)


extern ngx_http_header_t       ngx_http_headers_in[];
extern ngx_http_header_out_t   ngx_http_headers_out[];


#define ngx_http_set_log_request(log, r)                                      \
    ((ngx_http_log_ctx_t *) log->data)->current_request = r


#endif /* _NGX_HTTP_REQUEST_H_INCLUDED_ */
