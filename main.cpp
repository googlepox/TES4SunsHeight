
#include "obse/PluginAPI.h"
#include "common/IDebugLog.h"
#include <Hooks.h>
#include <Manager.h>

IDebugLog		gLog("Sun's Height.log");
PluginHandle	g_pluginHandle = kPluginHandle_Invalid;

OBSEMessagingInterface* g_messagingInterface{};
OBSEInterface* g_OBSEInterface{};
OBSECommandTableInterface* g_cmdTableInterface{};

// OBLIVION = Is not being compiled as a CS plugin.
#if OBLIVION
OBSEScriptInterface* g_script{};
OBSEStringVarInterface* g_stringInterface{};
OBSEArrayVarInterface* g_arrayInterface{};
OBSESerializationInterface* g_serializationInterface{};
OBSEConsoleInterface* g_consoleInterface{};
OBSEEventManagerInterface* g_eventInterface{};
#endif

static void SunsHeight_SaveCallback(void* reserved)
{
	_MESSAGE("Saving Sun's Height data...");
	g_serializationInterface->OpenRecord(SunsHeight::kSerializationSignature, 1);
	g_serializationInterface->WriteRecordData(&(SunsHeight::Manager::GetSingleton()->tempData.bodyTemp), sizeof(SunsHeight::Manager::GetSingleton()->tempData.bodyTemp));
	g_serializationInterface->WriteRecordData(&(SunsHeight::Manager::GetSingleton()->tempData.wetness), sizeof(SunsHeight::Manager::GetSingleton()->tempData.wetness));
	g_serializationInterface->WriteRecordData(&(SunsHeight::Manager::GetSingleton()->tempData.lastGameHour), sizeof(SunsHeight::Manager::GetSingleton()->tempData.lastGameHour));
	g_serializationInterface->WriteRecordData(&(SunsHeight::Manager::GetSingleton()->tempData.lastHeatLevel), sizeof(SunsHeight::Manager::GetSingleton()->tempData.lastHeatLevel));
	g_serializationInterface->WriteRecordData(&(SunsHeight::Manager::GetSingleton()->tempData.lastColdLevel), sizeof(SunsHeight::Manager::GetSingleton()->tempData.lastColdLevel));
	g_serializationInterface->WriteRecordData(&(SunsHeight::Manager::GetSingleton()->tempData.lastWetLevel), sizeof(SunsHeight::Manager::GetSingleton()->tempData.lastWetLevel));
	_MESSAGE("Sun's Height data saved sucessfully");
}

static void SunsHeight_LoadCallback(void* reserved)
{
	UInt32	type, version, length;

	
	while (g_serializationInterface->GetNextRecordInfo(&type, &version, &length))
	{
		switch (type)
		{
		case 'SUNH':
			_MESSAGE("Loading Sun's Height data...");
			g_serializationInterface->ReadRecordData(&(SunsHeight::Manager::GetSingleton()->tempData.bodyTemp), sizeof(SunsHeight::Manager::GetSingleton()->tempData.bodyTemp));
			g_serializationInterface->ReadRecordData(&(SunsHeight::Manager::GetSingleton()->tempData.wetness), sizeof(SunsHeight::Manager::GetSingleton()->tempData.wetness));
			g_serializationInterface->ReadRecordData(&(SunsHeight::Manager::GetSingleton()->tempData.lastGameHour), sizeof(SunsHeight::Manager::GetSingleton()->tempData.lastGameHour));
			g_serializationInterface->ReadRecordData(&(SunsHeight::Manager::GetSingleton()->tempData.lastHeatLevel), sizeof(SunsHeight::Manager::GetSingleton()->tempData.lastHeatLevel));
			g_serializationInterface->ReadRecordData(&(SunsHeight::Manager::GetSingleton()->tempData.lastColdLevel), sizeof(SunsHeight::Manager::GetSingleton()->tempData.lastColdLevel));
			g_serializationInterface->ReadRecordData(&(SunsHeight::Manager::GetSingleton()->tempData.lastWetLevel), sizeof(SunsHeight::Manager::GetSingleton()->tempData.lastWetLevel));
			_MESSAGE("Sun's Height data loaded sucessfully");
			_MESSAGE("Loaded SunsHeight save: temp=%f wet=%f",
				SunsHeight::Manager::GetSingleton()->tempData.bodyTemp,
				SunsHeight::Manager::GetSingleton()->tempData.wetness);
			break;
		default:
			break;
		}
	}
	
}

void MessageHandler(OBSEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case OBSEMessagingInterface::kMessage_LoadGame: {
		SunsHeight::Manager::GetSingleton()->Initialize();
		
	} break;
	default: break;
	}
}

bool OBSEPlugin_Query(const OBSEInterface* OBSE, PluginInfo* info)
{
	// fill out the info structure
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "SunsHeight";
	info->version = 1;

	// version checks
	if (OBSE->obseVersion < OBSE_VERSION_INTEGER)
	{
		_ERROR("OBSE version too old (got %08X expected at least %08X)", OBSE->obseVersion, OBSE_VERSION_PADDEDSTRING);
		return false;
	}

	if (!OBSE->isEditor)
	{
		if (OBSE->oblivionVersion < OBLIVION_VERSION_1_2_416)
		{
			_ERROR("incorrect runtime version (got %08X need at least %08X)", OBSE->oblivionVersion, OBLIVION_VERSION_1_2_416);
			return false;
		}
	}
	else
	{
		if (OBSE->editorVersion < CS_VERSION_1_2)
		{
			_ERROR("incorrect editor version (got %08X need at least %08X)", OBSE->editorVersion, CS_VERSION_1_2);
			return false;
		}
	}

	// version checks pass
	// any version compatibility checks should be done here
	return true;
}

bool OBSEPlugin_Load(OBSEInterface* OBSE)
{
	g_pluginHandle = OBSE->GetPluginHandle();

	// save the OBSE interface in case we need it later
	g_OBSEInterface = OBSE;

	OBSE->SetOpcodeBase(0x2770);

	// register to receive messages from OBSE
	g_messagingInterface = static_cast<OBSEMessagingInterface*>(OBSE->QueryInterface(kInterface_Messaging));
	g_messagingInterface->RegisterListener(g_pluginHandle, "OBSE", MessageHandler);

	if (!OBSE->isEditor)
	{
#if OBLIVION
		// script and function-related interfaces
		g_script = static_cast<OBSEScriptInterface*>(OBSE->QueryInterface(kInterface_Script));
		g_stringInterface = static_cast<OBSEStringVarInterface*>(OBSE->QueryInterface(kInterface_StringVar));
		g_arrayInterface = static_cast<OBSEArrayVarInterface*>(OBSE->QueryInterface(kInterface_ArrayVar));
		g_eventInterface = static_cast<OBSEEventManagerInterface*>(OBSE->QueryInterface(kInterface_EventManager));
		g_serializationInterface = static_cast<OBSESerializationInterface*>(OBSE->QueryInterface(kInterface_Serialization));
		g_consoleInterface = static_cast<OBSEConsoleInterface*>(OBSE->QueryInterface(kInterface_Console));

		g_serializationInterface->SetSaveCallback(g_pluginHandle, SunsHeight_SaveCallback);
		g_serializationInterface->SetLoadCallback(g_pluginHandle, SunsHeight_LoadCallback);
#endif
	}

	if (OBSE->GetPluginLoaded("RealTimeMenus"))
	{
		SunsHeight::Manager::GetSingleton()->realTimeMenusLoaded = true;
	}

	SunsHeight::Install();

	return true;
}
