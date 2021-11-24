// This ID is defined when the arduino
#define SECRET_PLANTER_ID  "test111"
//TODO: Before connect ardunio with planter module you need to define the ID
/* 아두이노 ID 작성 원리
	행정구역(동) 사용용도(store or farm) 재배기번호(1~)
For example,
 세곡동 매장 1번 모듈 => sgs1
 세곡동 농장 4번 모듈 => sgf4
 성복동 매장 2번 모듈 => sbs1
*/
#define SECRET_SSID "test"
#define SECRET_PASSWORD "testtest"
#define SECRET_HTTP_PORT 80

#define SECRET_HOST_NAME "ec67-210-99-13-35.ngrok.io" // hostname of web server:
#define SECRET_PATH_ROOT  "/"
#define SECRET_PATH_INIT  "/planters/init/"
