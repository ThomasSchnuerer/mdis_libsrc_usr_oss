/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!  
 *        \file  signal.c
 *
 *      \author  klaus.popp@men.de
 * 
 * 	   \project  MDISforLinux USR_OSS lib
 *
 *  	 \brief  Signal handling routines
 *
 *    \switches  none
 */
/*
 *---------------------------------------------------------------------------
 * Copyright 2000-2019, MEN Mikro Elektronik GmbH
 ******************************************************************************/
/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*! \page linuossigusage

  \section linuossigusagesect Using USR_OSS Signals on Linux

  See \ref uossigusagesect for a general description of USR_OSS
  signal handling.

  Under Linux, the native signal handling is used by means of \em signal()
  calls.

  Non-realtime signals are those <32. Those Signals will not be
  queued.  If the same signal arrives more than once when signals are
  masked, only one occurance of that signal will be reported to the
  user.  However if different signals arrive, the will be reported all
  (but only once each).

  Realtime signals *will* be queued. (Totally max. 1024 for
  entire system in Linux 2.2). 
  So it's recommended to use only these signals. \c UOS_SIG_USR1 and
  \c UOS_SIG_USR2 are defined as realtime signals.
*/


#include "uos_int.h"

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/

/*--------------------------------------+
|   TYPDEFS                             |
+--------------------------------------*/

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
/* signal handling is initialized */
static u_int32 G_sigInit;

/* user signal handler */
static void (*G_sigHandler)(u_int32 sigCode);

/* the installed signals */
static sigset_t G_installedSigs;

sigset_t G_noSigMask;

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void LocalSigHandler(int sigCode);

/**********************************************************************/
/** Init signal handling
 *
 * \copydoc usr_oss_specification.c::UOS_SigInit()
 *
 * \linux See \ref linuossigusagesect to see how signals handling is
 * implemented for Linux.
 *
 * \sa UOS_SigExit
 */
int32 UOS_SigInit(void (*sigHandler)(u_int32 sigCode))
{
	if (G_sigInit++ == 0 ){
		G_sigHandler = sigHandler; /* save user's signal handler */
	}
	else {
		errno = ERR_UOS_BUSY;
		return ERR_UOS_BUSY;
	}

	sigemptyset( &G_installedSigs );
	sigemptyset( &G_noSigMask );
	return(0);
}

/**********************************************************************/
/** Terminate signal handling
 *
 * \copydoc usr_oss_specification.c::UOS_SigExit()
 *
 * \linux See \ref linuossigusagesect to see how signals handling is
 * implemented for Linux.
 *
 * \sa UOS_SigInit
 */
int32 UOS_SigExit(void)
{
	if (!G_sigInit){
		errno = ERR_UOS_NOT_INIZED;
		return(ERR_UOS_NOT_INIZED);
	}
	--G_sigInit;
	return(0);
}

/**********************************************************************/
/** Install a signal to be received
 *
 * \copydoc usr_oss_specification.c::UOS_SigInstall()
 *
 * \linux See \ref linuossigusagesect to see how signals handling is
 * implemented for Linux.
 *
 * \linux No \c ERR_UOS_BUSY error is returned if signal already installed.
 * \sa UOS_SigRemove
 */
int32 UOS_SigInstall(u_int32 sigCode)
{
	struct sigaction sa;

	if (!G_sigInit){
		errno = ERR_UOS_NOT_INIZED;
		return(ERR_UOS_NOT_INIZED);
	}

	if( sigCode >= _NSIG ){
		errno = ERR_UOS_ILL_SIG;
		return ERR_UOS_ILL_SIG;
	}
	sa.sa_handler 	= LocalSigHandler; /* set the signal handler to call */
	sa.sa_mask 		= G_noSigMask;/* don't block other signals during
									 execution*/
	sa.sa_flags		= 0;		/* no special actions. Don't remove sig
								   handler after taking action */

	sigaction( sigCode, &sa, NULL );

	sigaddset( &G_installedSigs, sigCode );

	return 0;
}

/**********************************************************************/
/** Remove signal to be received
 *
 * \copydoc usr_oss_specification.c::UOS_SigRemove()
 *
 * \linux See \ref linuossigusagesect to see how signals handling is
 * implemented for Linux.
 *
 * \sa UOS_SigInstall
 */
int32 UOS_SigRemove(u_int32 sigCode)
{
	struct sigaction sa;

	if ( G_sigInit == 0){
		errno = ERR_UOS_NOT_INIZED;
		return(ERR_UOS_NOT_INIZED);
	}

	sa.sa_handler 	= SIG_DFL;	/* reset to default behaviour */
	sa.sa_mask 		= G_noSigMask;
	sa.sa_flags		= 0;

	sigaction( sigCode, &sa, NULL );

	sigdelset( &G_installedSigs, sigCode );

	return(0);
}

/**********************************************************************/
/** Mask all signals
 *
 * \copydoc usr_oss_specification.c::UOS_SigMask()
 *
 * \linux masks all signals installed by UOS_SigInstall()
 *
 * \linux See \ref linuossigusagesect to see how signals handling is
 * implemented for Linux.
 *
 * \sa UOS_SigUnMask
 */
int32 UOS_SigMask(void)
{
	if ( G_sigInit == 0){
		errno = ERR_UOS_NOT_INIZED;
		return(ERR_UOS_NOT_INIZED);
	}

	sigprocmask(SIG_BLOCK, &G_installedSigs, NULL);
	return(0);
}

/**********************************************************************/
/** Unmask all signals
 *
 * \copydoc usr_oss_specification.c::UOS_SigUnMask()
 *
 * \linux See \ref linuossigusagesect to see how signals handling is
 * implemented for Linux.
 *
 * \sa UOS_SigMask
 */
int32 UOS_SigUnMask(void)
{
	if ( G_sigInit == 0){
		errno = ERR_UOS_NOT_INIZED;
		return(ERR_UOS_NOT_INIZED);
	}

	sigprocmask(SIG_UNBLOCK, &G_installedSigs, NULL);
	return(0);
}

/**********************************************************************/
/** Wait until signal received
 *
 * \copydoc usr_oss_specification.c::UOS_SigWait()
 *
 * \linux \c ERR_UOS_ABORTED will never be returned. Any signal not
 * installed by UOS_SigInit will cause the standard signal handler to
 * be invoked.
 *
 * \linux See \ref linuossigusagesect to see how signals handling is
 * implemented for Linux.
 *
 * \sa UOS_SigInit, UOS_SigInstall
 */
int32 UOS_SigWait(u_int32 msec, u_int32 *sigCodeP)
{
	struct timespec ts;
	siginfo_t sinfo;
	int rv;

	if ( G_sigInit == 0){
		errno = ERR_UOS_NOT_INIZED;
		return(ERR_UOS_NOT_INIZED);
	}
/*
	DBGCMD(printf("UOS_SigWait: wait (msec=%d, ticks=%d) ..\n",
				  msec,ticks));
*/
	if( msec != 0 ){
		ts.tv_sec 	= msec / 1000;
		ts.tv_nsec 	= (msec % 1000) * 1000000;
	}
	else {
		ts.tv_sec	= 0x7FFFFFFF; /* not endless, but long */
		ts.tv_nsec	= 0;
	}

	rv = sigtimedwait( &G_installedSigs, &sinfo, &ts );
	/*printf( " sigtimedwait: rv=%d num=%d\n", rv, sinfo.si_signo );*/
	if( rv >= 0 ){
		*sigCodeP = rv;		/* rv contains arrived signal */
		LocalSigHandler( rv );	/* additionally call the signal handler */
		return 0;			/*  */
	}

	if( rv < 0 && msec ){
		errno = ERR_UOS_TIMEOUT;
		return(ERR_UOS_TIMEOUT);
	}
	errno = -1;
	return(-1);					/* timed out, but no timeout defined !?! */
}

/**********************************************************************/
/** Internal signal handler
 *
 * Checks if user signal handler and signal installed and jumps into
 * the user signal handler.
 *
 * If no user signal handler installed the signals are masked to be
 * sure LocalSigHandler is not called until next SigWait.
 *
 * \param sigCode		\IN signal code
 */
static void LocalSigHandler(int sigCode)/* nodoc */
{
	/*printf(">>> LocalSigHandler: sig=0x%04x\n",sigCode);*/

	if (G_sigHandler)					/* user handler installed ? */
		G_sigHandler(sigCode);			/* jump to it .. */
	else
		sigprocmask(SIG_BLOCK, &G_installedSigs, NULL);

}

