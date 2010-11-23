#include "rootpermission.h"
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include "Config..hxx"
void AssertRootPermission()
{
    if(getuid())
    {
        fprintf(stderr,"Need Root Permission\n");
#ifndef __DEBUG__
        SafeExit(1);
#endif
    }
}