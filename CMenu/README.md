# CMenu

 This is ready made class for creating and managing menus in Metamod

## Using

 * You need to allocate memory for your menu and pass a menu-handler it to the constructor
```cpp
void MenuHandle(CMenu *menu, edict_t *player, int item);

int main() {
	auto pMenu = new CMenu(MenuHandle);//allocate memory and pass a menu-handler
	
	//another code...
	return 0;
}
```

 * After that you need to install the header and add menu-items, after that may open menu
```cpp
void MenuHandle(CMenu *menu, edict_t *player, int item);

int main(edict_t *player) {
	auto pMenu = new CMenu(MenuHandle);//allocate memory and pass a menu-handler
	
	pMenu->menu_settitle("My menu title");
	pMenu->menu_additem(1/*alias*/, "Item 1"/*itemname*/);
	pMenu->menu_additem(2/*alias*/, "Item 2"/*itemname*/);
	pMenu->menu_additem(3/*alias*/, "Item 2"/*itemname*/);
	pMenu->menu_open(player);
	
	return 0;
}
```
<b>NOTE</b>: do not forget to deallocate memory for local menus in handler:
```cpp
void MenuHandle(CMenu *menu, edict_t *player, int item) {
	auto pItem = menu->menu_itemgetinfo(item);
	printf("select itemid = %i | itemalias = %i | itemname = %i", pItem->itemid,  pItem->itemalias_i, pItem->itemname);
	
	delete menu;//deallocate memory (menu destroy)
}
```

## [Full example of use](https://github.com/KaidoRen/Metamod-Utils/tree/master/CMenu/example)