/*
* ����: leehomwu
* ����: 2011-05-14
* �޶�: 2011-06-11, �����ִ������Ὣ����ʱ�� EIP Ҳ��ջ�����
* ����: ����δ��󡢸�����������������ʱ�ľ���λ�á�����·��
* ʹ��: �� main ���������ļ�������ͷ�ļ�����
* ʾ��: �������δ���򸡵����ʱ������STDOUT��ӡ����·���ϵ�ָ���ַ�����ƣ�
			signal[8] catched when running code at 80486f0
			signal[8] catched when running code at 80488ea
			signal[8] catched when running code at 80488d9
		
		��һ�����������У�����ָ���ַ:
			addr2line  80486f0 80488ea 80488d9 -s -C -f -e [��ִ���ļ�]

		�õ������
			main
			newsig.cpp:14
			a()
			kk.cpp:23
			b()
			kk.cpp:19
* �ڶ���
* ����: Jerry
* ����: 2014-08-29, ֧��ARM/MIPS������
*/


#ifndef __SEGV_CATCH_H
#define __SEGV_CATCH_H
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>

/* Default, Set ARCH to X86*/
#define __ARCH_X86

#ifdef __ARCH_X86
	#ifndef __USE_GNU
		#define __USE_GNU
		#include <ucontext.h>
		#include <sys/ucontext.h>
		#undef __USE_GNU
	#else
		#include <ucontext.h>
		#include <sys/ucontext.h>
	#endif
#elif defined(__ARCH_ARM)
	#include <asm/sigcontext.h>
	#include <ucontext.h>
	#include <sys/ucontext.h>
#elif defined(__ARCH_MIPS)
	#include <ucontext.h>
	#include <sys/ucontext.h>
#endif

#ifdef __cplusplus
	static void initSegvCatch(void);
    class C_SEGVCATCH{
		public:
			C_SEGVCATCH(){
				initSegvCatch();
			}
	};
	static C_SEGVCATCH C_segv_catch;
#else
	static void initSegvCatch(void) __attribute__ ((constructor));
#endif

static void OnSIGSEGV(int,siginfo_t*,void*);
static void initSegvCatch()
{
    struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_sigaction = OnSIGSEGV;
	act.sa_flags = SA_SIGINFO;
	if(sigaction(SIGSEGV, &act, NULL)<0 || sigaction(SIGFPE, &act, NULL)<0)
	{
		perror("sigaction:");
	}
}

static void OnSIGSEGV(int signum, siginfo_t *info, void *ptr)
{
	static int iTime;
	if (iTime++ >= 1){ /* �ݴ���������� ucontext_t �ṹ��ʱ������������÷�֧ */
		printf("ReEnter %s is not allowed!\n", __FUNCTION__);
		abort();
	}	

	void * array[25];
	int nSize = backtrace(array, sizeof(array)/sizeof(array[0]));
	int i;
	for (i=nSize-3; i>2; i--){ /* ͷβ������ַ������� */
		/* ��array����һ�£�ʹ��ַָ������ִ�еĴ��� */
		printf("signal[%d] catched when running code at %x\n", signum, (int)array[i] - 1);
	}
	
	if (NULL != ptr){
#ifdef __ARCH_X86
		ucontext_t* ptrUC = (ucontext_t*)ptr;
		int *pgregs = (int*)(&(ptrUC->uc_mcontext.gregs));
		int eip = pgregs[REG_EIP];
#elif defined(__ARCH_ARM)
		ucontext_t* ptrUC = (ucontext_t*)ptr;
		int eip = ptrUC->uc_mcontext.arm_pc;
#elif defined(__ARCH_MIPS)
		ucontext_t* ptrUC = (ucontext_t*)ptr;
		int eip = ptrUC->uc_mcontext.pc;
#endif
		if (eip != (int)array[i]){ /* �еĴ������Ὣ����ʱ�� EIP ��ջ */
			printf("signal[%d] catched when running code at %x\n", signum, (int)array[i] - 1);
		}
		printf("signal[%d] catched when running code at %x\n", signum, eip); /* ����ʱ��ָ���ַ */
	}else{
		printf("signal[%d] catched when running code at unknown address\n", signum);
	}

	abort();
}

#endif // __SEGV_CATCH_H

