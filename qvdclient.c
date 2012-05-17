#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include "qvdclient.h"

void help(const char *program)
{
  printf("%s [-?] [-d] -h host [-p port] -u username -w pass [-g wxh] [-f] \n", program);
  printf("  -? : shows this help\n");
  printf("  -d : Enables debugging\n");
  printf("  -h : indicates the host to connect to\n");
  printf("  -p : indicates the port to connect to, if not specified 8443 is used\n");
  printf("  -u : indicates the username for the connection\n");
  printf("  -w : indicates the password for the user\n");
  printf("  -g : indicates the geometry wxh. Example -g 1024x768\n");
  printf("  -f : Use fullscreen\n");
  printf("  -l : Use only list_of_vm (don't try to connect, useful for debugging)\n");
}

int parse_params(int argc, char **argv, const char **host, int *port, const char **user, const char **pass, const char **geometry, int *fullscreen, int *only_list_of_vm)
{
  int opt, error = 0;
  const char *program = argv[0];
  char *endptr;

  while ((opt = getopt(argc, argv, "?dh:p:u:w:g:fl")) != -1 )
    {
      switch (opt)
	{
	case '?':
	  help(program);
	  break;
	case 'd':
	  set_debug_level(2);
	  break;
	case 'h':
	  *host = optarg;
	  break;
	case 'p':
	  errno = 0;	  
	  *port = (int) strtol(optarg, &endptr, 10);
	  if ((errno == ERANGE && (*port == LONG_MAX || *port == LONG_MIN))
	      || optarg == endptr)
	      *port = -1;
	  break;
	case 'u':
	  *user = optarg;
	  break;
	case 'w':
	  *pass = optarg;
	  break;
	case 'g':
	  *geometry = optarg;
	  break;
	case 'f':
	  *fullscreen = 1;
	  break;
	case 'l':
	  *only_list_of_vm = 1;
	  break;
	default:
	  fprintf(stderr, "Parameter not recognized <%c>\n", opt);
	  error = 1;
	}
    }
  if (*host == NULL)
    {
      fprintf(stderr, "The host paramter -h is required\n");
      error = 1;
    }
  if (*user == NULL)
    {
      fprintf(stderr, "The user paramter -u is required\n");
      error = 1;
    }
  if (*pass == NULL)
    {
      fprintf(stderr, "The password paramter -w is required\n");
      error = 1;
    }
  if (*port < 1 || *port > 65535)
    {
      fprintf(stderr, "The port parameter must be between 1 and 65535\n");
      error = 1;
    }
  if (error)
    help(program);
  return error;
}


int main(int argc, char *argv[], char *envp[]) {
  qvdclient *qvd;
  const char *host = NULL, *user = NULL, *pass = NULL, *geometry = NULL;
  int port = 8443, fullscreen=0, vm_id, only_list_of_vm=0;
  if (parse_params(argc, argv, &host, &port, &user, &pass, &geometry, &fullscreen, &only_list_of_vm))
    return 1;

  qvd = qvd_init(host, port, user, pass);

  if (geometry)
    qvd_set_geometry(qvd, geometry);
  if (fullscreen)
    qvd_set_fullscreen(qvd);

  if (qvd_list_of_vm(qvd) == NULL)
    {
      printf("Error fetching vm for user %s in host %s\n", user, host);
      qvd_free(qvd);
      return 5;
    }
  if (qvd->numvms < 0)
    {
      printf("No vms found for user %s in host %s\n", user, host);
      qvd_free(qvd);
      return 2;
    }
  if (only_list_of_vm)
    {
      printf("No more acctions, -l has been specified\n");
      return 0;
    }
  /* TODO select VM, for now use the first one*/
  vm_id = qvd->vmlist->data->id;
  printf("Connecting to the first vm: vm_id %d\n", vm_id);
  qvd_connect_to_vm(qvd, vm_id);
  qvd_free(qvd);
  return 0;
}
