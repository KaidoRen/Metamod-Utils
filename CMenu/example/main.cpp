#include "amxxmodule.h"
#include "CMenu.h"
#include "memory.h"
#include "main.h"

hooks_t gl_messageBeginHandle = T_DEFAULT(MessageBegin);

bool OnAmxxAttach() {
	mem_add_hook(&gl_messageBeginHandle, g_engfuncs.pfnMessageBegin);

	return true;
}

void Menu_Handler(CMenu *pMenu, edict_t *pEdict, int iItem) {
	auto pItem = pMenu->menu_getiteminfo(iItem);

	switch (pItem->itemalias_i) {
	case 1:
		pEdict->v.frags = 0;
		//and reset deaths
		break;

	case 2:
		printf("CMenu version: %s\n", __CMENU_VERSION__);
		break;
	}

	delete pMenu; //menu destroy
}

int Menu_Callback(CMenu *pMenu, edict_t *pEdict, int iItem) {
	auto pItem = pMenu->menu_getiteminfo(iItem);
	int player = ENTINDEX(pEdict);

	switch (pItem->itemalias_i) {
	case 1:
		if (!g_amxxapi.IsPlayerAlive(player)) {//disable item for dead players
			pMenu->menu_setitemtempname("\\d%s", pItem->itemname);
			return ITEM_DISABLED;
		}
	}

	return ITEM_ENABLED;
}

void ClientCommand(edict_t *pEdict) {
	const char *szCmd = CMD_ARGV(0);
	const char *szArg = CMD_ARGV(1);

	int iIndex = ENTINDEX(pEdict);

	if (FStrEq(szCmd, "menuselect")) {
		gl_MenuManager.menu_select(pEdict, atoi(szArg));
		RETURN_META(MRES_IGNORED);
	}

	if (!FStrEq(szCmd, "say") && !FStrEq(szCmd, "say_team"))
		RETURN_META(MRES_IGNORED);

	if (FStrEq(szArg, "/menu")) {
		auto pMenu = new CMenu(Menu_Handler);

		pMenu->menu_settitle("Server menu");
		pMenu->menu_additem(1, Menu_Callback, "Reset Score");
		pMenu->menu_additem(2, "CMenu Info");
		pMenu->menu_setcontrolname(ControlNames::C_Exit, "Выход");

		pMenu->menu_open(pEdict);
		RETURN_META(MRES_SUPERCEDE);
	}

	RETURN_META(MRES_IGNORED);
}

void MessageBegin(int msg_dest, int msg_type, const float *pOrigin, edict_t *pEdict) {
	if (msg_type == MsgID_ShowMenu)
		gl_MenuManager.menu_reset(pEdict);

	mem_unpatch(&gl_messageBeginHandle);
	g_engfuncs.pfnMessageBegin(msg_dest, msg_type, pOrigin, pEdict);
	mem_patch(&gl_messageBeginHandle);
}