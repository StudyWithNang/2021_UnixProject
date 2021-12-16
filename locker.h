struct locker {
   int use; 	//0: empty, 1: full 
   int conn; 	//0: unconn, 1: conn
   int lock; 	//0: unlock, 1: lock
   char pw[10];
}