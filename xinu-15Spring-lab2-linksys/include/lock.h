#define READ		1
#define WRITE		2
#define DELETED		3

#define LFREE		4
#define	LUSED		5

#define NLOCKS		50

struct lentry{

	int 	lockState; 				/* To check if the lock is used or free */
	int 	lockHead;				/* index of head  */
	int 	lockTail;				/* index of tail */
	char	lockMode;				/* read or write mode */
	int		processList[NPROC];		/* Process(pid) currently holding the lock */
	int		lockDesp;				/* lock description */
	int		lockType;
	int		lockFlag;				/* if lock is in use or is not in use */
	int		lockPrio;				/* lock priority */

};

#define isbadlock(l) (l<0 || l>=NLOCKS)

extern  struct 		lentry locktab[];
extern 	int			nextInLine; 

extern 	int 		linit(void);
extern 	int 		lcreate(void);
extern  void 		increasePriority(int lock);
extern 	int			lock (int ldes1, int type, int priority);
extern 	int 		releaseall (int numlocks, int arguments,... );
extern 	int 		ldelete(int lockdescriptor);
extern  int			freeAllLocks(int pid);
extern	void		toNext(int lockId);
extern	void		modifyThePrio(int lockId);
extern	void		changeOnRelease(int pid);




