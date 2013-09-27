#include "erl_driver.h"
#include "ei.h"

int erts_printf(const char *, ...);

/*
** Open a port
*/
static ErlDrvData trace_file_start(ErlDrvPort port, char *buff)
{
  erts_printf("start\n");
  return NULL;
}

/*
** Close a port
*/
static void trace_file_stop(ErlDrvData handle)
{
  erts_printf("close\n");
}

void do_call(char *pid, int index, char *buf) {
  char m[MAXATOMLEN];
  char f[MAXATOMLEN];
  long a;
  int arity;

  if(ei_decode_tuple_header(buf, &index, &arity) || (arity != 3))
    return;
  if(ei_decode_atom(buf, &index, m))
    return;
  if(ei_decode_atom(buf, &index, f))
    return;
  
  if(ei_decode_long(buf, &index, &a)) {
    if(ei_decode_list_header(buf, &index, &arity))
      return;
    a = arity;
  }

  erts_printf("%s call %s/%s/%d\n", pid, m, f, a);
}

void do_return_from(char *pid, int index, char *buf) {
}

void dispatch(char *pid, char *type, int index, char *buf) {
  if(strcmp("call", type) == 0)
    do_call(pid, index, buf);
  else if(strcmp("return_from", type) == 0)
    do_return_from(pid, index, buf);
}

/*
** Data sent from erlang to port.
*/
static void trace_file_output(ErlDrvData handle, char *buf,
			      ErlDrvSizeT len)
{
  int index = 1;
  ei_print_term(stdout, buf, &index);
  erts_printf("\n");
  /* erts_printf("output: %d: %u\n", rc, (unsigned char)buff[0]); */
  int arity;
  char atom[MAXATOMLEN];
  char type[MAXATOMLEN];
  char pidstr[100];
  index = 1;
  erlang_pid pid;
  if(!ei_decode_tuple_header(buf, &index, &arity)) {
    if(!ei_decode_atom(buf, &index, atom)) {
      if(strcmp("trace", atom) == 0) {
        if(!ei_decode_pid(buf, &index, &pid)) {
          if(!ei_decode_atom(buf, &index, type)) {
            snprintf(pidstr, 100, "<0.%d.%d>", pid.num, pid.serial);
            erts_printf("%s: %s\n", pidstr, type);
            dispatch(pidstr, type, index, buf);
          }
        }
      }
    }
  }
}

/*
** Control message from erlang, we handle $f, which is flush.
*/
static ErlDrvSSizeT trace_file_control(ErlDrvData handle,
                                       unsigned int command, 
                                       char* buff, ErlDrvSizeT count, 
                                       char** res, ErlDrvSizeT res_size)
{
    erts_printf("control\n");
    if (command == 'f') {
        return 1;
    } 
    return -1;
}

/*
** Driver unloaded
*/
static void trace_file_finish(void)
{
  erts_printf("finish\n");
}

ErlDrvEntry dt_trace_driver_entry = {
    NULL,		   /* F_PTR init, N/A */
    trace_file_start,      /* L_PTR start, called when port is opened */
    trace_file_stop,       /* F_PTR stop, called when port is closed */
    trace_file_output,     /* F_PTR output, called when erlang has sent */
    NULL,                  /* F_PTR ready_input, called when input descriptor 
			      ready */
    NULL,                  /* F_PTR ready_output, called when output 
			      descriptor ready */
    "dt_trace",            /* char *driver_name, the argument to open_port */
    trace_file_finish,     /* F_PTR finish, called when unloaded */
    NULL,                  /* void * that is not used (BC) */
    trace_file_control,    /* F_PTR control, port_control callback */
    NULL,                  /* F_PTR timeout, driver_set_timer callback */
    NULL,                  /* F_PTR outputv, reserved */
    NULL, /* ready_async */
    NULL, /* flush */
    NULL, /* call */
    NULL, /* event */
    ERL_DRV_EXTENDED_MARKER,
    ERL_DRV_EXTENDED_MAJOR_VERSION,
    ERL_DRV_EXTENDED_MINOR_VERSION,
    0,
    NULL,
    NULL,
    NULL,
};

/*
** Driver initialization routine
*/
DRIVER_INIT(dt_trace)
{
    return &dt_trace_driver_entry;
}
