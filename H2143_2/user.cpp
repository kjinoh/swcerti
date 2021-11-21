#include <list>
#include <set>
#include <unordered_map>

using namespace std;

#define MAX_REQ 20010
#define MAX_LINE 501
#define MAX_EQUIP 501
#define MAX_TIME 500001

struct REQ_t {
	int tstamp;
	int pid;
	int lid;
	int eid;
	int time;
	int status;  // 1 : 대기중, 2 : 생산중, 3 : 완료
	int endtime;	// 정렬기준. 초기값은 500001
	int heapIdx;
	void init() { time = 0; pid = -1; lid = -1; eid = -1; time = 0; status = 0; endtime = MAX_TIME; heapIdx = -1; }
	// 끝나는 시간이 작은 순으로 heap 에서 관리. 끝나는 시간이 같으면 생산라인 작은 순
	bool operator<(REQ_t& b)
	{
		return endtime == b.endtime ? lid < b.lid : endtime < b.endtime;
	}
};
REQ_t req[MAX_REQ];
int idx_req;

// 라인은 먼저 들어온 요청 순으로 처리
struct LINE_t{
	int status;	// 0: idle, 1 : pending, 2: working
	list<REQ_t*> wReq;
	void init() { status = 0; wReq.clear(); }
};
LINE_t line[MAX_LINE];

// 장비는 생산 라인 작은 순으로 배정.
struct CompareL {
	bool operator()(const REQ_t* a, const REQ_t* b) const
	{
		return a->lid == b->lid ? a->tstamp < b->tstamp : a->lid < b->lid;
	}
};
struct EQUIP_t {
	int status;	// 0: idle, 1 : working
	REQ_t* rReq;
	set<REQ_t*, CompareL> wReq;
	void init() { status = 0; wReq.clear(); }
};	
EQUIP_t equip[MAX_EQUIP];

unordered_map<int, int> hashProd;	// pId, req id

REQ_t* heap[MAX_REQ];
int heapSize = 0;

void heapInit(void)
{
	heapSize = 0;
}

void heapUpdate(int index)
{
	int current = index;
	while (current > 0 && *heap[current] < *heap[(current - 1) / 2])
	{
		REQ_t* temp = heap[(current - 1) / 2];
		heap[(current - 1) / 2] = heap[current];
		heap[(current - 1) / 2]->heapIdx = (current - 1) / 2;
		heap[current] = temp;
		heap[current]->heapIdx = current;
		current = (current - 1) / 2;
	}
}

int heapPush(REQ_t* value)
{
	if (heapSize + 1 > MAX_REQ)
	{
		return 0;
	}

	heap[heapSize] = value;
	heap[heapSize]->heapIdx = heapSize;

	int current = heapSize;
	while (current > 0 && *heap[current] < *heap[(current - 1) / 2])
	{
		REQ_t* temp = heap[(current - 1) / 2];
		heap[(current - 1) / 2] = heap[current];
		heap[(current - 1) / 2]->heapIdx = (current - 1) / 2;
		heap[current] = temp;
		heap[current]->heapIdx = current;
		current = (current - 1) / 2;
	}

	heapSize = heapSize + 1;

	return 1;
}

int heapPop(REQ_t** value)
{
	if (heapSize <= 0)
	{
		return -1;
	}

	*value = heap[0];
	heapSize = heapSize - 1;

	heap[0] = heap[heapSize];
	heap[0]->heapIdx = 0;

	int current = 0;
	while (current * 2 + 1 < heapSize)
	{
		int child;
		if (current * 2 + 2 == heapSize)
		{
			child = current * 2 + 1;
		}
		else
		{
			child = *heap[current * 2 + 1] < *heap[current * 2 + 2] ? current * 2 + 1 : current * 2 + 2;
		}

		if (*heap[current] < *heap[child])
		{
			break;
		}

		REQ_t* temp = heap[current];
		heap[current] = heap[child];
		heap[current]->heapIdx = current;
		heap[child] = temp;
		heap[child]->heapIdx = child;

		current = child;
	}
	return 1;
}

void init(int L, int M) {
	idx_req = 0;
	for (register int i = 0; i < L; i++)
	{
		line[i].init();
	}
	for (register int i = 0; i < M; i++)
	{
		equip[i].init();
	}
	for (register int i = 0; i < MAX_REQ; i++)
	{
		req[i].init();
	}
	return;
}

void update(int tStamp)
{
	int ret;
	while(1)
	{
		REQ_t* r;
		ret = heapPop(&r);
		if (ret == -1)
			return;

		if (r->status == 2) // 요청이 생산중이면
		{
			// 생산완료 시간이 tStamp 이전이면
			if (r->endtime <= tStamp)
			{
				r->status = 3;
				line[r->lid].status = 0;
				line[r->lid].wReq.pop_front();
				equip[r->eid].status = 0;
				equip[r->eid].rReq = nullptr;
				//equip[r->eid].wReq.erase(*equip[r->eid].wReq.begin());
				REQ_t* lnextReq = nullptr;
				REQ_t* enextReq = nullptr;

				// 풀린 라인에 걸려있는 다음 작업이 필요로 하는 장비에 라인 너머 추가
				if (line[r->lid].wReq.size() > 0)
				{
					auto it = line[r->lid].wReq.begin();
					lnextReq = *it;
					equip[lnextReq->eid].wReq.insert(lnextReq);
				}

				// 풀린 장비에 걸려있는 다음 작업을 진행할 수 있는지 체크
				if (equip[r->eid].wReq.size() > 0)
				{
					auto it = equip[r->eid].wReq.begin();
					enextReq = *it;
					if (enextReq->status == 1)
					{
						if (line[enextReq->lid].status == 0 || line[enextReq->lid].status == 1)
						{
							enextReq->endtime = r->endtime + enextReq->time;
							enextReq->status = 2;
							heapUpdate(enextReq->heapIdx);
							line[enextReq->lid].status = 2;
							equip[enextReq->eid].status = 1;
							equip[enextReq->eid].rReq = enextReq;
							equip[enextReq->eid].wReq.erase(enextReq);
						}
					}
				}

				//풀린 라인에 걸려있는 다음 작업을 진행할 수 있는지 체크
				if (line[r->lid].wReq.size() > 0)
				{
					auto it = line[r->lid].wReq.begin();
					lnextReq = *it;
					if (equip[lnextReq->eid].status == 0)
					{
						lnextReq->endtime = r->endtime + lnextReq->time;
						lnextReq->status = 2;
						heapUpdate(lnextReq->heapIdx);
						line[lnextReq->lid].status = 2;
						equip[lnextReq->eid].status = 1;
						equip[lnextReq->eid].rReq = lnextReq;
						equip[lnextReq->eid].wReq.erase(lnextReq);
					}
					else
					{
						line[r->lid].status = 1;	// pending
						equip[lnextReq->eid].wReq.insert(lnextReq);
					}
				}

			}
			else
			{
				heapPush(r);
				return;
			}
		}
		else if (r->status == 1)	// 요청이 대기중이면
		{
			heapPush(r);
			return;
		}
		else if (r->status == 0)
		{
			
		}
	}
}

int request(int tStamp, int pId, int mLine, int eId, int mTime) {
	update(tStamp - 1);

	idx_req++;
	REQ_t* r = &req[idx_req];
	r->tstamp = tStamp; 	r->pid = pId;	r->lid = mLine; 	r->eid = eId; 	r->time = mTime;
	if (line[r->lid].status == 0 && equip[r->eid].status == 0)
	{
		r->endtime = tStamp + r->time;
		r->status = 2;
		line[r->lid].status = 2;
		equip[r->eid].status = 1;
		equip[r->eid].rReq = r;
		//equip[r->eid].wReq.insert(r);
	}
	else
	{
		r->status = 1;
		if (equip[r->eid].status != 0 && line[r->lid].status == 0)	// 장비가 돌고 있는 경우
		{
			line[r->lid].status = 1;
			equip[r->eid].wReq.insert(r);
		}
	}
	heapPush(r);
	line[mLine].wReq.push_back(r);
	
	hashProd[pId] = idx_req;

	update(tStamp);
	if (line[mLine].status == 0 || line[mLine].status == 1)	// idle, pending
		return -1;
	else if (line[mLine].status == 2)	// working
	{
		auto it = line[mLine].wReq.begin();
		REQ_t* lineReq = *it;
		return  lineReq->pid;
	}
	return -1;
}

int status(int tStamp, int pId) {
	update(tStamp);
	auto it = hashProd.find(pId);
	if (it == hashProd.end())
	{
		return 0;
	}
	int reqId = it->second;
	return req[reqId].status;
}
