/*
*	CMenu library
*	Copyright 2017 Kaido Ren.
*
*	Credits: s1lent, In-line
*
*	https://github.com/KaidoRen/Metamod-Utils/blob/master/CMenu/ - CMenu the official repository.
*
*
*	CMenu library is free software; you can redistribute it and/or modify it
*	under the terms of the GNU General Public License as published by the
*	Free Software Foundation; either version 2 of the License, or (at
*	your option) any later version.
*
*	CMenu library is distributed in the hope that it will be useful, but
*	WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*	General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with CMenu library; if not, see <http://www.gnu.org/licenses/>.
*
*	In addition, as a special exception, the author gives permission to
*	link the code of CMenu library with the Half-Life Game Engine ("HL
*	Engine") and Modified Game Libraries ("MODs") developed by Valve,
*	L.L.C ("Valve").  You must obey the GNU General Public License in all
*	respects for all of the code used other than the HL Engine and MODs
*	from Valve.  If you modify this file, you may extend this exception
*	to your version of the file, but you are not obligated to do so.  If
*	you do not wish to do so, delete this exception statement from your
*	version.
*/

#ifndef __CMENU_VERSION__
#define __CMENU_VERSION__ "1.2.0"

#ifdef WIN32
#pragma once
#endif

using namespace std;

#ifndef MAX_CLIENTS
#define MAX_CLIENTS 32
#endif

#define Invalid_MenuID -1
#define Offset_MenuID 205
#define MsgID_ShowMenu 96

#define MENU_ITEMS_PER_PAGE 7
#define MENU_ITEMS_PER_PAGE_WITHOUT_CONTROLS 9/*+ back and + next*/

#define MENU_EXIT 10
#define MENU_BACK 8
#define MENU_NEXT 9

/* Menu keys */
#define MENU_KEY_1		(1<<0)
#define MENU_KEY_2		(1<<1)
#define MENU_KEY_3		(1<<2)
#define MENU_KEY_4		(1<<3)
#define MENU_KEY_5		(1<<4)
#define MENU_KEY_6		(1<<5)
#define MENU_KEY_7		(1<<6)
#define MENU_KEY_8		(1<<7)
#define MENU_KEY_9		(1<<8)
#define MENU_KEY_0		(1<<9)
#define MENU_KEYS_ALL	(MENU_KEY_1|MENU_KEY_2|MENU_KEY_3|MENU_KEY_4|MENU_KEY_5|MENU_KEY_6|MENU_KEY_7|MENU_KEY_8|MENU_KEY_9|MENU_KEY_0)

class CMenu;
typedef void(*MHANDLE)(CMenu *, edict_t*, int);
typedef int(*MCALLBACK)(CMenu *, edict_t*, int);

typedef enum {
	C_Exit,
	C_Back,
	C_Next,

	C_End
} ControlNames;

typedef enum {
	ITEM_ENABLED,
	ITEM_DISABLED
} CallbackValues;

typedef struct {
	int			menuid;
	int			menupage;
	int			keystorealitem[MENU_ITEMS_PER_PAGE_WITHOUT_CONTROLS];
} MenuPlayerStruct;

typedef struct {
	CMenu		*menu;
	int			menuid;
	MHANDLE		handle;
} MenuStruct;

typedef struct {
	char		itemname[128];
	char *		itemalias_c;
	int			itemalias_i;
	int			itemid;
	MCALLBACK	callback;
} MenuItemsStruct;

typedef vector<MenuStruct *> MenuStructVector;
typedef vector<MenuItemsStruct *> MenuItemsStructVector;

class CMenuManager {
public:
	CMenuManager(void) {}

	void	menu_select(edict_t *edict, int key);
	bool	menu_find_by_id(const int id);
	MenuStruct * menu_find_by_id_s(const int id);
	void	menu_reset(edict_t *edict);
};

class CMenu {
public:
	CMenu(MHANDLE handle);

	bool	menu_open(edict_t *edict);
	void	menu_settitle(const char *title, ...);

	int		menu_additem(char *alias, const char * name, ...);
	int		menu_additem(int alias, const char * name, ...);
	int		menu_additem(char * alias, MCALLBACK callback, const char * name, ...);
	int		menu_additem(int alias, MCALLBACK callback, const char * name, ...);

	int		menu_getitems();

	void	menu_numformat(char *fmt);
	void	menu_setcontrolname(ControlNames control, const char * name);
	void	menu_setcallback(const int item, MCALLBACK callback);
	void	menu_setitemtempname(const char *name, ...);
	MenuItemsStruct * menu_getiteminfo(const int id);

	~CMenu();
private:
	void	menu_destroy();

	int		m_iMenuID;
	string	m_sTitle;
	string	m_sTempName;
	string	m_sNumberFormat;
	string	m_sControlNames[C_End];
	MHANDLE m_pHandle;
	MenuItemsStructVector m_vecItems;
};

extern CMenuManager gl_MenuManager;
extern MenuPlayerStruct gl_PlayerMenu[MAX_CLIENTS + 1];
extern MenuStructVector gl_vecMenus;

inline void UTIL_ShowMenu(edict_t *pEdict, int slots, int time, char *menu, int mlen) {
	char *n = menu;
	char c = 0;
	int a;

	while (*n) {
		a = mlen;

		if (a > 175)
			a = 175;

		mlen -= a;
		c = *(n += a);
		*n = 0;

		MESSAGE_BEGIN(MSG_ONE, MsgID_ShowMenu, NULL, pEdict);
		WRITE_SHORT(slots);
		WRITE_CHAR(time);
		WRITE_BYTE(c ? TRUE : FALSE);
		WRITE_STRING(menu);
		MESSAGE_END();

		*n = c;
		menu = n;
	}
}

#endif