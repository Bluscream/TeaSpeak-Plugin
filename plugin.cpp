#ifdef _WIN32
#pragma warning (disable : 4100)
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"
#include "plugin.h"

static struct TS3Functions ts3Functions;

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#define PLUGIN_API_VERSION 22

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128

static char* pluginID = NULL;

#ifdef _WIN32
static int wcharToUtf8(const wchar_t* str, char** result) {
	int outlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0);
	*result = (char*)malloc(outlen);
	if(WideCharToMultiByte(CP_UTF8, 0, str, -1, *result, outlen, 0, 0) == 0) {
		*result = NULL;
		return -1;
	}
	return 0;
}
#endif

const char* ts3plugin_name() {
#ifdef _WIN32
	static char* result = NULL;
	if(!result) {
		const wchar_t* name = L"TeaSpeak";
		if(wcharToUtf8(name, &result) == -1) {
			result = "TeaSpeak";
		}
	}
	return result;
#else
	return "TeaSpeak";
#endif
}

const char* ts3plugin_version() {
    return "1.0";
}

int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

const char* ts3plugin_author() {
    return "Bluscream";
}

const char* ts3plugin_description() {
    return "Plugin to integrate TeaSpeak.";
}

void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
    ts3Functions = funcs;
}
int ts3plugin_init() {
    return 0;
}

void ts3plugin_shutdown() {
	if(pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}
void ts3plugin_registerPluginID(const char* id) {
	const size_t sz = strlen(id) + 1;
	pluginID = (char*)malloc(sz * sizeof(char));
	_strcpy(pluginID, sz, id);
}

void ts3plugin_currentServerConnectionChanged(uint64 serverConnectionHandlerID) {
    printf("PLUGIN: currentServerConnectionChanged %llu (%llu)\n", (long long unsigned int)serverConnectionHandlerID, (long long unsigned int)ts3Functions.getCurrentServerConnectionHandlerID());
}

const char* ts3plugin_infoTitle() {
	return "TeaSpeak";
}

void ts3plugin_freeMemory(void* data) {
	free(data);
}

int ts3plugin_requestAutoload() {
	return 0;
}

static struct PluginMenuItem* createMenuItem(enum PluginMenuType type, int id, const char* text, const char* icon) {
	struct PluginMenuItem* menuItem = (struct PluginMenuItem*)malloc(sizeof(struct PluginMenuItem));
	menuItem->type = type;
	menuItem->id = id;
	_strcpy(menuItem->text, PLUGIN_MENU_BUFSZ, text);
	_strcpy(menuItem->icon, PLUGIN_MENU_BUFSZ, icon);
	return menuItem;
}

#define BEGIN_CREATE_MENUS(x) const size_t sz = x + 1; size_t n = 0; *menuItems = (struct PluginMenuItem**)malloc(sizeof(struct PluginMenuItem*) * sz);
#define CREATE_MENU_ITEM(a, b, c, d) (*menuItems)[n++] = createMenuItem(a, b, c, d);
#define END_CREATE_MENUS (*menuItems)[n++] = NULL; assert(n == sz);

enum {
	MENU_ID_CHANNEL_1 = 1,
	MENU_ID_GLOBAL_1,
	MENU_ID_GLOBAL_2
};

void ts3plugin_initMenus(struct PluginMenuItem*** menuItems, char** menuIcon) {
	BEGIN_CREATE_MENUS(3);
	CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_CHANNEL, MENU_ID_CHANNEL_1, "List Musicbots", "");
	CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_1, "List Musicbots", "");
	CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_2, "List Supported Formats", "");
	END_CREATE_MENUS;
	*menuIcon = (char*)malloc(PLUGIN_MENU_BUFSZ * sizeof(char));
	_strcpy(*menuIcon, PLUGIN_MENU_BUFSZ, "teaspeak.png");
}

void ts3plugin_onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID) {
	anyID ownID;
	ts3Functions.getClientID(serverConnectionHandlerID, &ownID);
	uint64 ownCID;
	ts3Functions.getChannelOfClient(serverConnectionHandlerID, ownID, &ownCID);
	switch(type) {
		case PLUGIN_MENU_TYPE_GLOBAL:
			switch(menuItemID) {
				case MENU_ID_GLOBAL_1:
					ts3Functions.requestSendChannelTextMsg(serverConnectionHandlerID, ".mbot list server", ownCID, NULL);
					break;
				case MENU_ID_GLOBAL_2:
					ts3Functions.requestSendChannelTextMsg(serverConnectionHandlerID, ".mbot formats", ownCID, NULL);
					break;
				default:
					break;
			}
			break;
		case PLUGIN_MENU_TYPE_CHANNEL:
			switch(menuItemID) {
				case MENU_ID_CHANNEL_1:
					ts3Functions.requestClientMove(serverConnectionHandlerID, ownID, selectedItemID, "123", NULL);
					ts3Functions.requestSendChannelTextMsg(serverConnectionHandlerID, ".mbot list", selectedItemID, NULL);
					ts3Functions.requestClientMove(serverConnectionHandlerID, ownID, ownCID, "123", NULL);
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}