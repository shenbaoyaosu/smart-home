#ifndef ADDR_MGR_H
#define ADDR_MGR_H

int AddrMgr_Add(const char* cmd, const char* addr);
char* AddrMgr_Find(const char* cmd);
void AddrMgr_Remove(const char* cmd);
void AddrMgr_Clear();

#endif
