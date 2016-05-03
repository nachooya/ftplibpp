#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include "ftplib.h"

typedef struct {
 
  uint64_t start_time;
  uint64_t bytes;
  
} ftp_status_t;

uint64_t timestampMsec () {

    long            ms; // Milliseconds
    struct timespec spec;
    clock_gettime (CLOCK_REALTIME, &spec);

    ms = spec.tv_sec*1000 + round(spec.tv_nsec / 1.0e6); // 
    
    return ms;
    
}

void printUsage (const char* exec) 
{
    printf ("Usage:\n\n%s ftp://user:password@server/path/file\n\n", exec);  
}

int ftpCallbackXfer (off64_t xfered, void *p_status) 
{  
    ftp_status_t* status = (ftp_status_t*) p_status;
    status->bytes = xfered;
    if (status->start_time == 0) {
      status->start_time = timestampMsec();
    }
    else {
        
      uint64_t elapsed = (timestampMsec() - status->start_time) / 1000;
        
      if (elapsed > 0) {
        uint64_t speed = (xfered/1024) / elapsed;    
        printf ("\rGot %ld bytes @ %lu KB/s", xfered, speed);
      }
    }
    
    return 1;
}


int main (int argc, char* argv[])
{
    if (argc < 2) {
      printUsage(argv[0]);
      return 1;
    }
    
    ftp_status_t status;
        
    char* user = NULL;
    char* pass = NULL;
    char* server = NULL;
    char* path = NULL;
    
    if (sscanf (argv[1], "ftp://%a[^:]:%a[^@]@%a[^/]%as", &user, &pass, &server, &path) != 4) {
      printUsage(argv[0]);
      return 1;
    }
  
    printf ("User: %s\n", user);
    printf ("Pass: %s\n", pass);
    printf ("Server: %s\n", server);
    printf ("Path: %s\n", path);
      
    ftplib *ftp = new ftplib();
    
    ftp->SetCallbackXferFunction(&ftpCallbackXfer);
    ftp->SetCallbackBytes(10*1024);
    ftp->SetCallbackArg(&status);
    
    ftp->Connect(server);
    ftp->Login(user, pass);
    
    status.start_time = 0;
    status.bytes = 0;
    
    int res = ftp->Get("downloaded.tmp", path, ftplib::image);
    printf ("Download result: %d\n", res);

    ftp->Quit();
    return 0;
}
