#include <stdio.h>
#include <event2/event.h>



int main(void){
	struct event_base* epbase;
//初始化一个event_base 结构体

	epbase = event_base_new();

	if(epbase == NULL){
		perror("event_base_new:");
		exit(1);
	}





//释放创建出来的结构体
	event_base_free(epbase);
	return 0;
}
