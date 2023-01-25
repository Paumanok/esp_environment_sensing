/*
request.h
requests taken from example code and modified

*/



void http_post_single(const char* post_req);
int http_get_single();
int parse_get_resp(char* buf, int buf_size);

//static void http_post_task(void *pvParameters);
//static void http_get_task(void *pvParameters);