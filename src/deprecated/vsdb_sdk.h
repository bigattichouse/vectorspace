#ifdef __cplusplus
     #define cppfudge "C"
#else
     #define cppfudge
#endif

#ifdef BUILD_DLL
     // the dll exports
     #define EXPORT __stdcall __declspec(dllexport)
#else
     // the exe imports
    // #define EXPORT extern cppfudge __stdcall  __declspec(dllimport)
    #define EXPORT 
#endif

typedef struct {
  vs_cursor *cursor;
  vs_cursor *thesaurus;
  vs_queue  *output_buffer;
  interpreter_session *session;
} vs_cloudspace;
 

EXPORT vs_cloudspace *create_cloudspace();
EXPORT void destroy_cloudspace(vs_cloudspace **cloudspace);
EXPORT int vsdb_execute(vs_cloudspace *cloudspace,char command,char *element,float value);
EXPORT int vsdb_outputbuffer(vs_cloudspace *cloudspace,char *buffer);
EXPORT int vsdb_savecloudspace(vs_cloudspace *cloudspace,char *data,char *thesaurus);
EXPORT int vsdb_loadcloudspace(vs_cloudspace *cloudspace,char *data,char *thesaurus);
EXPORT int vsdb_metacluster(vs_cloudspace *cloudspace,int segment,int segmentsize,float threshold,int cluster_minimum_size);
