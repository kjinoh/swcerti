#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>

extern void init(int L, int M);
extern int request(int tStamp, int pId, int mLine, int eId, int mTime);
extern int status(int tStamp, int pId);

/////////////////////////////////////////////////////////////////////////

#define CMD_INIT 1
#define CMD_REQUEST 2
#define CMD_STATUS 3

static bool run() {
	int q;
	scanf("%d", &q);

	int l, m, tstamp, pid, mline, eid, mtime;
	int cmd, ans, ret = 0;
	bool okay = false;

	for (int i = 0; i < q; ++i) {
		scanf("%d", &cmd);
		switch (cmd) {
		case CMD_INIT:
			scanf("%d %d", &l, &m);
			init(l, m);
			okay = true;
			break;
		case CMD_REQUEST:
			scanf("%d %d %d %d %d %d", &tstamp, &pid, &mline, &eid, &mtime, &ans);
			ret = request(tstamp, pid, mline, eid, mtime);
			if (ans != ret)
				okay = false;
			break;
		case CMD_STATUS:
			scanf("%d %d %d", &tstamp, &pid, &ans);
			ret = status(tstamp, pid);
			if (ans != ret)
				okay = false;
			break;
		default:
			okay = false;
			break;
		}
	}
	return okay;
}

int main() {
	setbuf(stdout, NULL);
	freopen("sample_input.txt", "r", stdin);

	int T, MARK;
	scanf("%d %d", &T, &MARK);

	for (int tc = 1; tc <= T; tc++) {
		int score = run() ? MARK : 0;
		printf("#%d %d\n", tc, score);
	}

	return 0;
}
