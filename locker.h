struct locker {
   int use; //0: empty, 1: full 
   int conn; //0: unconn, 1: conn
   char pw[10];
}
