



typedef struct  {
    uint  D1;
    ushort  D2;   //ushort           unsigned short int
    ushort  D3;   //ushort
    char   D4[8];
} t_uuid;

int CreateGUID(t_uuid *guid);
char *GuidToString(t_uuid GUID);
int IsEqualGuid(t_uuid GUID1,t_uuid GUID2);
t_uuid StringToHash(char *HASH);
char *HashToString(t_uuid GUID);
int IsNullGuid(t_uuid *guid);
int CompareGuids(t_uuid GUID1,t_uuid GUID2);
t_uuid NullGuid();
