#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include "port.h"
#include "uuid.h"

#ifdef WIN32

#include <windows.h>

#endif


void *libuuidhandle;


typedef struct  {
    uint    D1;
    ushort  D2;
    ushort  D3;
    uint    D4;
    uint    D5;
} c_uuid;



int IsNullGuid(t_uuid *guid){
  c_uuid *CMP;
  CMP = (c_uuid *)guid;
  if (
  (CMP->D1==0) &&
  (CMP->D2==0) &&
  (CMP->D3==0) &&
  (CMP->D4==0) &&
  (CMP->D5==0)
  ){return(1);} else {return(0);}
}

t_uuid NullGuid(){
 t_uuid guid;
 //c_uuid *CMP;
 //CMP = (c_uuid *) &guid;
 //CMP->D1=0; CMP->D2=0;
 //CMP->D3=0; CMP->D4=0;
 //CMP->D5=0;
 memset (&guid,0,sizeof(t_uuid));
 return(guid);
}

int CreateGUID(t_uuid *guid) {
  c_uuid *CMP;
#ifdef WIN32
  //windows uses built in GUID generation
#else
  int fd;
#endif
  CMP = (c_uuid *)guid;
  CMP->D1=0; CMP->D2=0;
  CMP->D3=0; CMP->D4=0;
  CMP->D5=0;
  guid->D1=0;guid->D2=0;

#ifdef WIN32
   CoCreateGuid((GUID *)guid);

#else
/*

  int (*uuid_generate_time)(t_uuid *guid);
  char *error;
    libuuidhandle = dlopen ("libuuid.so.1", RTLD_LAZY);
    if (!libuuidhandle) {
      //  fprintf (stderr, "%s\n", dlerror());
        exit(1);
    }
    uuid_generate_time = dlsym(libuuidhandle, "uuid_generate_time");
    if ((error = dlerror()) != NULL)  {
       // fprintf (stderr, "%s\n", error);
        exit(1);
    }
    uuid_generate_time(guid);
*/      
	fd = open("/dev/urandom", O_RDONLY, 0); //
	if(fd >= 0)
	{
		read(fd, guid, 16);
		close(fd);
	}

//    dlclose(libuuidhandle);
#endif
   return(0);
}

int inline IsEqualGuid2(t_uuid GUID1,t_uuid GUID2){
  c_uuid *CMP1,*CMP2;
  CMP1 = (c_uuid *) &GUID1;
  CMP2 = (c_uuid *) &GUID2;
  return ((GUID1.D1==GUID2.D1) &&
      (GUID1.D2==GUID2.D2) &&
      (GUID1.D3==GUID2.D3) &&
      (CMP1->D4==CMP2->D4) &&
      (CMP1->D5==CMP2->D5));
}


int inline IsEqualGuid(t_uuid GUID1,t_uuid GUID2){
  int result; /* i */
  c_uuid *CMP1,*CMP2;
  result=0;

  CMP1 = (c_uuid *) &GUID1;
  CMP2 = (c_uuid *) &GUID2;

  if (GUID1.D1==GUID2.D1){
    if (GUID1.D2==GUID2.D2){
      if (GUID1.D3==GUID2.D3){
       if (CMP1->D4==CMP2->D4){
        if (CMP1->D5==CMP2->D5){result=1;}
       }
      }
    }
  }

  return(result);
}

int inline CompareGuids(t_uuid GUID1,t_uuid GUID2){
  int result; /* i */
  c_uuid *CMP1,*CMP2;
  c_uuid GT,LT;
//  t_uuid *R;
//  char *s;
  result=0;
  CMP1 = (c_uuid *) &GUID1;
  CMP2 = (c_uuid *) &GUID2;
  GT.D1 = (GUID1.D1 > GUID2.D1);
  GT.D2 = (GUID1.D2 > GUID2.D2);
  GT.D3 = (GUID1.D3 > GUID2.D3);
  GT.D4 = ((uint)CMP1->D4 > (uint)CMP2->D4);
  GT.D5 = ((uint)CMP1->D5 > (uint)CMP2->D5);

  LT.D1 = (GUID1.D1 < GUID2.D1);
  LT.D2 = (GUID1.D2 < GUID2.D2);
  LT.D3 = (GUID1.D3 < GUID2.D3);
  LT.D4 = ((uint)CMP1->D4 < (uint)CMP2->D4);
  LT.D5 = ((uint)CMP1->D5 < (uint)CMP2->D5);

//  printf("\n%d %d %d %d %d\n",RES.D1,RES.D2,RES.D3,RES.D4,RES.D5);
/*
  R = (t_uuid *)&RES;
  s= GuidToString(*R);
  printf("%s\n",s);
  free(s);
*/

 if (IsEqualGuid(GUID1,GUID2)){return(0);}


 result = -1;
 if (GT.D1 == 0){if (LT.D1==0){
   if (GT.D2 == 0){if (LT.D2==0){
     if (GT.D3 == 0){if (LT.D3==0){
       if (GT.D4 == 0){if (LT.D4==0){
          if (GT.D5 == 0){if (LT.D5==0){
            result=-1;
          }} else {result=1;}
       }} else {result=1;}
     }} else {result=1;}
   }} else {result=1;}
 }} else {result=1;}

  return(result);
}



char *GuidToString(t_uuid GUID){
 char *result;
 unsigned char c0,c1,c2,c3,c4,c5,c6,c7 ;
 c0 =GUID.D4[0];c4 =GUID.D4[4];
 c1 =GUID.D4[1];c5 =GUID.D4[5];
 c2 =GUID.D4[2];c6 =GUID.D4[6];
 c3 =GUID.D4[3];c7 =GUID.D4[7];

 result = malloc(38);
 sprintf(result,"{%.8x-%.4x-%.4x-%02x%02x-%02x%02x%02x%02x%02x%02x}",   // do not localize
    GUID.D1, GUID.D2, GUID.D3, c0,c1,c2,c3,c4,c5,c6,c7) ;
 return(result);
}

char *HashToString(t_uuid GUID){
 char *result;
 unsigned char c0,c1,c2,c3,c4,c5,c6,c7 ;
 c0 =GUID.D4[0];c4 =GUID.D4[4];
 c1 =GUID.D4[1];c5 =GUID.D4[5];
 c2 =GUID.D4[2];c6 =GUID.D4[6];
 c3 =GUID.D4[3];c7 =GUID.D4[7];
 result = malloc(33);
 sprintf(result,"%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",GUID.D1, GUID.D2, GUID.D3, c0,c1,c2,c3,c4,c5,c6,c7) ;
 result[32]='\0';
 return(result);
}

t_uuid StringToHash(char *HASH){
 char *result;
 t_uuid GUID;
 unsigned int D1,D2,D3,c0,c1,c2,c3,c4,c5,c6,c7 ;
 result = malloc(33);
 result[32]='\0';
 sscanf(HASH,"%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",   // do not localize
    &D1, &D2, &D3,&c0,&c1,&c2,&c3,&c4,&c5,&c6,&c7);
 GUID.D1=D1; GUID.D2=D2; GUID.D3=D3;
 GUID.D4[0]=c0; GUID.D4[1]=c1;
 GUID.D4[2]=c2; GUID.D4[3]=c3;
 GUID.D4[4]=c4; GUID.D4[5]=c5;
 GUID.D4[6]=c6; GUID.D4[7]=c7;
 free(result);
 return(GUID);
}

