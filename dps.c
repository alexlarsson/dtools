#include <proc/readproc.h>

#include "dtools.h"

int
main (int argc, char *argv[])
{
  PROCTAB *proct;
  proc_t *proc_info;
  GVariant *value;
  DtoolsVariantWriter *writer;
  GVariantBuilder builder, builder2;
  GString *string;
  int i;
  time_t now;

  now = time (NULL);
  
  g_type_init ();

  writer = dtools_variant_writer_new (STDOUT_FILENO);
  
  proct = openproc (PROC_FILLARG | PROC_FILLSTAT | PROC_FILLSTATUS | PROC_FILLUSR);

  while ((proc_info = readproc (proct,NULL)))
    {
      char char_as_str[2] = { 0, 0 };

      g_variant_builder_init (&builder, G_VARIANT_TYPE_VARDICT);

      value = g_variant_new_uint32 (proc_info->tid); 
      g_variant_builder_add(&builder, "{sv}", "pid", value);
      
      value = g_variant_new_uint32 (proc_info->ppid); 
      g_variant_builder_add(&builder, "{sv}", "ppid", value);

      value = g_variant_new_uint32 (proc_info->euid);
      g_variant_builder_add(&builder, "{sv}", "euid", value);

      value = g_variant_new_uint32 (proc_info->egid);
      g_variant_builder_add(&builder, "{sv}", "egid", value);

      value = g_variant_new_string (proc_info->euser); 
      g_variant_builder_add(&builder, "{sv}", "user", value);

      if (proc_info->cmd != NULL)
	{
	  value = g_variant_new_string (proc_info->cmd); 
	  g_variant_builder_add(&builder, "{sv}", "cmd", value);
	}
      
      if (proc_info->cmdline != NULL)
	{
	  string = g_string_new ("");
	  for (i = 0; proc_info->cmdline[i] != NULL; i++)
	    {
	      if (i != 0)
		g_string_append_c (string, ' ');
	      g_string_append (string, proc_info->cmdline[i]);
	    }
      
	  value = g_variant_new_string (g_string_free (string, FALSE));
	  g_variant_builder_add(&builder, "{sv}", "cmdline", value);
	  
	  g_variant_builder_init (&builder2, G_VARIANT_TYPE_STRING_ARRAY);
	  for (i = 0; proc_info->cmdline[i] != NULL; i++)
	      g_variant_builder_add(&builder2, "s", proc_info->cmdline[i]);
	  value = g_variant_builder_end (&builder2);
	  g_variant_builder_add(&builder, "{sv}", "cmdvec", value);
	}
      else if (proc_info->cmd != NULL)
	{
	  char *s = g_strconcat ("[", proc_info->cmd, "]", NULL);
	  value = g_variant_new_string (s);
	  g_free (s);
	  g_variant_builder_add(&builder, "{sv}", "cmdline", value);
	}
      
      char_as_str[0] = proc_info->state;
      value = g_variant_new_string (char_as_str); 
      g_variant_builder_add(&builder, "{sv}", "state", value);
      
      value = g_variant_new_uint64 (proc_info->utime); 
      g_variant_builder_add(&builder, "{sv}", "utime", value);

      value = g_variant_new_uint64 (proc_info->stime); 
      g_variant_builder_add(&builder, "{sv}", "stime", value);

      value = g_variant_new_uint64 (proc_info->cutime); 
      g_variant_builder_add(&builder, "{sv}", "cutime", value);

      value = g_variant_new_uint64 (proc_info->cstime); 
      g_variant_builder_add(&builder, "{sv}", "cstime", value);
      
      value = g_variant_new_uint64 (now - proc_info->start_time); 
      g_variant_builder_add(&builder, "{sv}", "time", value);
      
      value = g_variant_new_uint64 (proc_info->start_time); 
      g_variant_builder_add(&builder, "{sv}", "start", value);

      value = g_variant_new_uint64 (proc_info->vm_size); 
      g_variant_builder_add(&builder, "{sv}", "vsize", value);

      value = g_variant_new_uint64 (proc_info->vm_rss); 
      g_variant_builder_add(&builder, "{sv}", "rss", value);

      value = g_variant_builder_end (&builder);
      
      dtools_variant_writer_add (writer, value);
    }
  closeproc (proct);
  return 0;
}
 
