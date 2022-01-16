# 2021_UnixProject  

## Project  
* 유닉스 시스템 수업시간에 배운 내용을 응용하여 사물함 관리 시스템을 구현하는 프로젝트 이다.  
* Server에서는 사물함을 관리하는 역할을 수행하고, Client에서는 사물함 관리 시스템의 고객 역할을 수행한다.  
* Server와 Client가 통신을 할 수 있도록 소켓을 사용하였고 여러 개(명)의 Client를 처리하기 위해 쓰레드를 사용하였다.  

## Flowchart  


### How to compile
1. final_server.c   
```
gcc -o server final_server.c -lpthread D_REENTRANT
```

2. final_client.c   
```
  gcc -o client final_client.c -lpthread D_REENTRANT   
````


### 2021/12/17 complete
