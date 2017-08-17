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

#include "precompiled.h"

CMenuManager		gl_MenuManager;
MenuPlayerStruct	gl_PlayerMenu[MAX_CLIENTS + 1];
MenuStructVector	gl_vecMenus;

/* CMenuManager Class Start */

void CMenuManager::menu_select(edict_t * edict, int key) {
	int player = ENTINDEX(edict);

	if (gl_PlayerMenu[player].menuid < 0)
		return;

	int *menuid = &gl_PlayerMenu[player].menuid;

	if (!menu_find_by_id(*menuid))
		return;

	if (auto menu = menu_find_by_id_s(*menuid)) {
		auto menu_class = menu->menu;

		bool pagination = bool(menu_class->menu_getitems() > MENU_ITEMS_PER_PAGE_WITHOUT_CONTROLS);

		switch (key) {
		case MENU_EXIT:
			*menuid = Invalid_MenuID;
			gl_PlayerMenu[player].menupage = 0;
			break;
		case MENU_BACK:
			if (pagination) {
				--gl_PlayerMenu[player].menupage;
				menu_class->menu_open(edict);
			}
			else {
				*menuid = Invalid_MenuID;
				gl_PlayerMenu[player].menupage = 0;
				menu->handle(menu_class, edict, gl_PlayerMenu[player].keystorealitem[key]);
			}
			break;
		case MENU_NEXT:
			if (pagination) {
				++gl_PlayerMenu[player].menupage;
				menu_class->menu_open(edict);
			}
			else {
				*menuid = Invalid_MenuID;
				gl_PlayerMenu[player].menupage = 0;
				menu->handle(menu_class, edict, gl_PlayerMenu[player].keystorealitem[key]);
			}
			break;
		default:
			*menuid = Invalid_MenuID;
			gl_PlayerMenu[player].menupage = 0;
			menu->handle(menu_class, edict, gl_PlayerMenu[player].keystorealitem[key]);
			break;
		}
	}
}

void CMenuManager::menu_reset(edict_t *edict) {
	if (!edict->pvPrivateData)
		return;

	int player = ENTINDEX(edict);

	if (player > 0 && player <= gpGlobals->maxClients)
		gl_PlayerMenu[player].menuid = Invalid_MenuID;
}

bool CMenuManager::menu_find_by_id(const int id) {
	auto it = gl_vecMenus.begin();

	while (it != gl_vecMenus.end()) {
		auto menu = *it;

		if (menu->menuid == id)
			return true;

		it++;
	}
	return false;
}

MenuStruct * CMenuManager::menu_find_by_id_s(const int id) {
	auto it = gl_vecMenus.begin();

	while (it != gl_vecMenus.end()) {
		auto menu = *it;

		if (menu->menuid == id)
			return menu;

		it++;
	}
	return nullptr;
}

/* CMenu Class Start */

CMenu::CMenu(MHANDLE handle) {
	if (gl_vecMenus.empty()) {
		auto menu = new MenuStruct;
		gl_vecMenus.push_back(menu);//dummy value
	}

	auto menu = new MenuStruct;
	menu->menu = this;
	menu->handle = m_pHandle = handle;
	menu->menuid = m_iMenuID = gl_vecMenus.size();

	gl_vecMenus.push_back(menu);

	m_sNumberFormat = "\\r%d.";
	m_sControlNames[C_Back] = "Back";
	m_sControlNames[C_Next] = "Next";
	m_sControlNames[C_Exit] = "Exit";
}

bool CMenu::menu_open(edict_t * edict) {
	if (gl_vecMenus.empty() || m_vecItems.empty())
		return false;

	if (!edict->pvPrivateData)
		return false;

	int player = ENTINDEX(edict);
	int page = gl_PlayerMenu[player].menupage;

	if (page < 0)
		return false;

	char buffer[512];
	int len, bitsKeys = MENU_KEY_0, retvalue;
	int items = m_vecItems.size(), start = 0, end = 0, pages = 0, i = 0, valid = 0;
	int items_per_page = items > MENU_ITEMS_PER_PAGE_WITHOUT_CONTROLS ? MENU_ITEMS_PER_PAGE : MENU_ITEMS_PER_PAGE_WITHOUT_CONTROLS;/*back and exit*/

	if ((start = page * items_per_page) >= items)
		start = page = gl_PlayerMenu[player].menupage = 0;

	if ((end = start + items_per_page) > items)
		end = items;

	pages = items / items_per_page + (items % items_per_page ? 1 : 0);

	if (pages == 1)
		len = snprintf(buffer, sizeof(buffer), "\\y%s\n\n", m_sTitle.c_str());
	else
		len = snprintf(buffer, sizeof(buffer), "\\y%s \\d(%d/%d)\n\n", m_sTitle.c_str(), page + 1, pages);

	auto it = m_vecItems.begin() + start;

	while (start < end && it != m_vecItems.end()) {
		auto item = *it++;
		start++;

		retvalue = ITEM_ENABLED;
		m_sTempName.clear();

		if (item->callback)
			retvalue = item->callback(this, edict, start - 1);

		retvalue == ITEM_DISABLED ? bitsKeys &= ~(1 << valid) : bitsKeys |= (1 << valid);
		len += snprintf(buffer + len, sizeof(buffer) - len, m_sNumberFormat.c_str(), ++valid);
		len += snprintf(buffer + len, sizeof(buffer) - len, "\\w %s\n%s", m_sTempName.empty() ? item->itemname : m_sTempName.c_str(), start == end ? "\n" : "");

		gl_PlayerMenu[player].keystorealitem[valid] = start - 1;
	}

	if (items > items_per_page) {
		page ? bitsKeys |= MENU_KEY_8 : bitsKeys &= ~MENU_KEY_8;
		(end < items) ? bitsKeys |= MENU_KEY_9 : bitsKeys &= ~MENU_KEY_9;
		len += snprintf(buffer + len, sizeof(buffer) - len, m_sNumberFormat.c_str(), MENU_BACK);
		len += snprintf(buffer + len, sizeof(buffer) - len, "%s %s\n", page ? "\\w" : "\\d", m_sControlNames[C_Back].c_str());
		len += snprintf(buffer + len, sizeof(buffer) - len, m_sNumberFormat.c_str(), MENU_NEXT);
		len += snprintf(buffer + len, sizeof(buffer) - len, "%s %s\n", end < items ? "\\w" : "\\d", m_sControlNames[C_Next].c_str());
	}

	len += snprintf(buffer + len, sizeof(buffer) - len, m_sNumberFormat.c_str(), C_Exit);
	len += snprintf(buffer + len, sizeof(buffer) - len, "\\w %s", m_sControlNames[C_Exit].c_str());

	UTIL_ShowMenu(edict, bitsKeys, -1, buffer, len);
	gl_PlayerMenu[player].menuid = m_iMenuID;

	return true;
}

void CMenu::menu_destroy() {
	if (gl_vecMenus.empty())
		return;

	auto it = gl_vecMenus.begin();

	while (it != gl_vecMenus.end()) {
		auto menu = *it;

		if (menu->menuid == m_iMenuID) {
			gl_vecMenus.erase(it);
			m_vecItems.clear();
			delete menu;
			return;
		}

		it++;
	}
}

void CMenu::menu_settitle(const char * title, ...) {
	va_list argptr;
	char buffer[128];

	va_start(argptr, title);
	vsprintf(buffer, title, argptr);
	va_end(argptr);

	m_sTitle = buffer;
}

int CMenu::menu_additem(char *alias, const char * name, ...) {
	va_list argptr;
	char buffer[128];

	va_start(argptr, name);
	vsprintf(buffer, name, argptr);
	va_end(argptr);

	auto pMenuItem = new MenuItemsStruct;

	strcpy(pMenuItem->itemname, buffer);
	pMenuItem->itemalias_c = alias;
	pMenuItem->callback = nullptr;
	pMenuItem->itemid = m_vecItems.size();

	m_vecItems.push_back(pMenuItem);
	return pMenuItem->itemid;
}

int CMenu::menu_additem(int alias, const char * name, ...) {
	va_list argptr;
	char buffer[128];

	va_start(argptr, name);
	vsprintf(buffer, name, argptr);
	va_end(argptr);

	auto pMenuItem = new MenuItemsStruct;

	strcpy(pMenuItem->itemname, buffer);
	pMenuItem->itemalias_i = alias;
	pMenuItem->callback = nullptr;
	pMenuItem->itemid = m_vecItems.size();

	m_vecItems.push_back(pMenuItem);
	return pMenuItem->itemid;
}

int CMenu::menu_additem(char * alias, MCALLBACK callback, const char * name, ...) {
	va_list argptr;
	char buffer[128];

	va_start(argptr, name);
	vsprintf(buffer, name, argptr);
	va_end(argptr);

	auto pMenuItem = new MenuItemsStruct;

	strcpy(pMenuItem->itemname, buffer);
	pMenuItem->itemalias_c = alias;
	pMenuItem->callback = callback;
	pMenuItem->itemid = m_vecItems.size();

	m_vecItems.push_back(pMenuItem);
	return pMenuItem->itemid;
}

int CMenu::menu_additem(int alias, MCALLBACK callback, const char * name, ...) {
	va_list argptr;
	char buffer[128];

	va_start(argptr, name);
	vsprintf(buffer, name, argptr);
	va_end(argptr);

	auto pMenuItem = new MenuItemsStruct;

	strcpy(pMenuItem->itemname, buffer);
	pMenuItem->itemalias_i = alias;
	pMenuItem->callback = callback;
	pMenuItem->itemid = m_vecItems.size();

	m_vecItems.push_back(pMenuItem);
	return pMenuItem->itemid;
}

int CMenu::menu_getitems() {
	return m_vecItems.size();
}

void CMenu::menu_numformat(char * fmt) {
	m_sNumberFormat = fmt;
}

void CMenu::menu_setcontrolname(ControlNames control, const char * name) {
	m_sControlNames[control] = name;
}

void CMenu::menu_setcallback(const int id, MCALLBACK callback) {
	if (m_vecItems.empty())
		return;

	auto it = m_vecItems.begin() + id;

	if (it >= m_vecItems.end())
		return;

	auto item = *it;

	item->callback = callback;
}

void CMenu::menu_setitemtempname(const char * name, ...) {
	va_list argptr;
	char buffer[128];

	va_start(argptr, name);
	vsprintf(buffer, name, argptr);
	va_end(argptr);

	m_sTempName = buffer;
}

MenuItemsStruct * CMenu::menu_getiteminfo(const int id) {
	if (m_vecItems.empty())
		return nullptr;

	auto it = m_vecItems.begin() + id;

	if (it >= m_vecItems.end())
		return nullptr;

	auto item = *it;
	return item;
}

CMenu::~CMenu() {
	menu_destroy();
}