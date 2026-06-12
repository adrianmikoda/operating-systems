#pragma once

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

extern void sig_default(); 
extern void sig_mask();
extern void sig_ignore();
extern void sig_handle(); 
