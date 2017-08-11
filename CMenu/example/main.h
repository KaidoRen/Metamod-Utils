#pragma once

bool OnAmxxAttach();
extern void ClientCommand(edict_t *pEdict);
extern void MessageBegin(int msg_dest, int msg_type, const float *pOrigin, edict_t *pEdict);
extern void Menu_Handler(CMenu *pMenu, edict_t *pEdict, int iItem);
extern int Menu_Callback(CMenu *pMenu, edict_t *pEdict, int iItem);