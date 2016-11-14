#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <arpa/inet.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

static void
echo_read_cb(struct bufferevent *bev, void *ctx)
{
    //如果客户端有数据写过来，那么会触发当前的回调函数

    /* This callback is invoked when there is data to read on bev. */
    //struct evbuffer *input = bufferevent_get_input(bev);
    //input 就是当前bufferevent的输入缓冲区地址，如果想得到用户端数据
    //就从input中去获取

    //struct evbuffer *output = bufferevent_get_output(bev);
    //output 就是当前bufferevent的输出缓冲区地址，如果想向客户端写数据
    //就将数据写到output中就可以了

    //input 拿出来， 

    //小->大
	struct evbuffer evb;
	memset(evb,0,sizeof(evb));

	int ret = bufferevent_read_buffer(bev,&evb);
	int i= 0;
	for(i = 0; evb.buffer[i] !='\0';i++){
		evb.buffer[i] = toupper(evb.buffer[i]);
	}
	
	ret = bufferevent_write_buffer(bev,&evb);

    /* Copy all the data from the input buffer to the output buffer. */
//    evbuffer_add_buffer(output, input);
}

static void
echo_event_cb(struct bufferevent *bev, short events, void *ctx)
{
    if (events & BEV_EVENT_ERROR)
        perror("Error from bufferevent");
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        bufferevent_free(bev);
    }
}

static void
accept_conn_cb(struct evconnlistener *listener,
        evutil_socket_t fd, struct sockaddr *address, int socklen,
        void *ctx)
{
    //fd---服务器已经accept成功的cfd fd 就是可以直接跟客户端 通信的套接字

    /* We got a new connection! Set up a bufferevent for it. */
    struct event_base *base = evconnlistener_get_base(listener);

    //创建一个bufferevent 绑定fd 和base 
    struct bufferevent *bev = bufferevent_socket_new(
            base, fd, BEV_OPT_CLOSE_ON_FREE);

    //当前刚创建好的bufferevent事件 注册一些回调函数
    bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);


    //启动监听bufferevnet的 读事件 和 写事件
    bufferevent_enable(bev, EV_READ|EV_WRITE);
}

static void
accept_error_cb(struct evconnlistener *listener, void *ctx)
{
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Got an error %d (%s) on the listener. "
            "Shutting down.\n", err, evutil_socket_error_to_string(err));

    event_base_loopexit(base, NULL);
}

int
main(int argc, char **argv)
{
    struct event_base *base;
	//时间监听
    struct evconnlistener *listener;

    struct sockaddr_in sin;

    int port = 9876;

    if (argc > 1) {
        port = atoi(argv[1]);
    }
    if (port<=0 || port>65535) {
        puts("Invalid port");
        return 1;
    }

    //创建一个eventbase句柄，在内核开辟一个监听事件的根节点
    base = event_base_new();
    if (!base) {
        puts("Couldn't open event base");
        return 1;
    }

    /* Clear the sockaddr before using it, in case there are extra
     * platform-specific fields that can mess us up. */
    memset(&sin, 0, sizeof(sin));
    /* This is an INET address */
    sin.sin_family = AF_INET;
    /* Listen on 0.0.0.0 */
    sin.sin_addr.s_addr = htonl(0);
    /* Listen on the given port. */
    sin.sin_port = htons(port);

    //将 listen_fd 封装一个事件，默认在里面已经执行了bind 和linsten指令
    listener = evconnlistener_new_bind(base, accept_conn_cb, NULL,
            LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
            (struct sockaddr*)&sin, sizeof(sin));
    if (!listener) {
        perror("Couldn't create listener");
		
        return 1;
    }

    evconnlistener_set_error_cb(listener, accept_error_cb);



    //开启循环监听事件
    event_base_dispatch(base);

    return 0;
}
