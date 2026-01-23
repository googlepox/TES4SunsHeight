#include "Manager.h"
#include <GameAPI.h>
#include <ModTable.h>

namespace SunsHeight
{
    static TESGlobal* g_bodyTempGlobal = nullptr;
    static TESGlobal* g_ambientTempGlobal = nullptr;
    static TESGlobal* g_coverageGlobal = nullptr;
    static TESGlobal* g_warmthGlobal = nullptr;
    static TESGlobal* g_wetnessGlobal = nullptr;
    static TESGlobal* g_fireHeatGlobal = nullptr;
    static TESGlobal* g_nearFireGlobal = nullptr;
    static TESGlobal* g_showBarsGlobal = nullptr;

    float WET_GAIN_RAIN = 4.0f;
    float WET_GAIN_SNOW = 2.0f;
    float WET_DRY_OUT = 1.5f;
    float WET_DRY_IN = 2.0f;
    float WET_DRY_FIRE = 6.0f;

    float SLOT_CHEST = 1.0f;
    float SLOT_LEGS = 0.8f;
    float SLOT_HEAD = 0.6f;
    float SLOT_FEET = 0.4f;
    float SLOT_HANDS = 0.4f;
    float SLOT_RING = 0.1f;
    float SLOT_AMULET = 0.1f;

    float BODY_TEMP_BASE = 50.0f;
    float AMBIENT_TEMP_BASE = 50.0f;

    // Cold thresholds
    float COLD_1 = 40.0f;
    float COLD_2 = 25.0f;
    float COLD_3 = 10.0f;
    float COLD_4 = 2.0f;

    // Heat thresholds
    float HOT_1 = 60.0f;
    float HOT_2 = 75.0f;
    float HOT_3 = 90.0f;
    float HOT_4 = 98.0f;

    // Wetness thresholds
    float WET_1 = 25.0f;
    float WET_2 = 60.0f;
    float WET_3 = 90.0f;

    float RATE_PER_SECOND = 0.05f;

    float RESPONSE_RATE = 0.01f;
    float WET_CHANGE = 0.0f;

    float HEAVY_ARMOR_COVERAGE = 1.5f;
    float LIGHT_ARMOR_COVERAGE = 1.2f;
    float CLOTHING_WEIGHT_MULT = 1.0f;
    float ARMOR_RATING_MULT = 1.0f;
    float CLOTHING_COVERAGE = 1.0f;

    float MAX_FIRE_DIST = 800.0f;
    float HEAT_MULT = 20.0f;
    float SMALL_FIRE_FACTOR = 1.2f;
    float MEDIUM_FIRE_FACTOR = 1.5f;
    float LARGE_FIRE_FACTOR = 1.5f;
    
    
    float WEATHER_SNOW_TEMP = -15.0f;
    float WEATHER_RAIN_TEMP = -5.0f;
    float WEATHER_CLOUDY_TEMP = 5.0f;
    float WEATHER_PLEASANT_TEMP = 10.0f;
    float WEATHER_DEFAULT_TEMP = 10.0f;

    float GROUND_NORMAL_TEXTURE = 5.0f;
    float GROUND_SNOW_TEXTURE = -10.0f;
    float GROUND_DESERT_TEXTURE = 10.0f;
    float GROUND_ASH_TEXTURE = 10.0f;

    float NIGHT_TEMP = -5.0;

    float ALTITUDE_PENALTY = 3000.0f;

    float DEBUG_MODE = 0.0f;
    
    float DEATH = 1.0f;

    float SHOW_BARS = 1.0f;

    float MAX_AMB_TEMP_CHANGE_PER_SEC = 1.5f;
    float MAX_AMB_TEMP_CHANGE_FAST = 8.0f;

    float SMOOTHING = 0.02f;

    static std::vector<TESObjectREFR*> cachedFires;
    static float lastFireScanTime = -9999.0f;

    std::unordered_map<std::string, float*> configMap =
    {
        // Wetness rates
        { "WET_GAIN_RAIN",   &WET_GAIN_RAIN },
        { "WET_GAIN_SNOW",   &WET_GAIN_SNOW },
        { "WET_DRY_OUT",     &WET_DRY_OUT },
        { "WET_DRY_IN",      &WET_DRY_IN },
        { "WET_DRY_FIRE",    &WET_DRY_FIRE },

        // Slot weights
        { "SLOT_CHEST",      &SLOT_CHEST },
        { "SLOT_LEGS",       &SLOT_LEGS },
        { "SLOT_HEAD",       &SLOT_HEAD },
        { "SLOT_FEET",       &SLOT_FEET },
        { "SLOT_HANDS",      &SLOT_HANDS },
        { "SLOT_RING",       &SLOT_RING },
        { "SLOT_AMULET",     &SLOT_AMULET },

        // Base body temperature
        { "BODY_TEMP_BASE",  &BODY_TEMP_BASE },

        // Base ambient temperature
        { "AMBIENT_TEMP_BASE",  &AMBIENT_TEMP_BASE },

        // Cold thresholds
        { "COLD_1",          &COLD_1 },
        { "COLD_2",          &COLD_2 },
        { "COLD_3",          &COLD_3 },
        { "COLD_4",          &COLD_4 },

        // Heat thresholds
        { "HOT_1",           &HOT_1 },
        { "HOT_2",           &HOT_2 },
        { "HOT_3",           &HOT_3 },
        { "HOT_4",           &HOT_4 },

        // Wetness thresholds
        { "WET_1",           &WET_1 },
        { "WET_2",           &WET_2 },
        { "WET_3",           &WET_3 },

        // Temperature update rate
        { "RATE_PER_SECOND", &RATE_PER_SECOND },

        { "RESPONSE_RATE", &RESPONSE_RATE },


        { "HEAVY_ARMOR_COVERAGE", &HEAVY_ARMOR_COVERAGE },
        { "LIGHT_ARMOR_COVERAGE", &LIGHT_ARMOR_COVERAGE },
        { "CLOTHING_WEIGHT_MULT", &CLOTHING_WEIGHT_MULT },
        { "ARMOR_RATING_MULT", &ARMOR_RATING_MULT },
        { "CLOTHING_COVERAGE", &CLOTHING_COVERAGE },

        { "HEAT_MULT", &HEAT_MULT },
        { "MAX_FIRE_DIST", &MAX_FIRE_DIST },
        { "SMALL_FIRE_FACTOR", &SMALL_FIRE_FACTOR },
        { "MEDIUM_FIRE_FACTOR", &MEDIUM_FIRE_FACTOR },
        { "LARGE_FIRE_FACTOR", &LARGE_FIRE_FACTOR },

        { "WEATHER_SNOW_TEMP", &WEATHER_SNOW_TEMP },
        { "WEATHER_RAIN_TEMP", &WEATHER_RAIN_TEMP },
        { "WEATHER_CLOUDY_TEMP", &WEATHER_CLOUDY_TEMP },
        { "WEATHER_PLEASANT_TEMP", &WEATHER_PLEASANT_TEMP },
        { "WEATHER_DEFAULT_TEMP", &WEATHER_DEFAULT_TEMP },


        { "GROUND_NORMAL_TEXTURE", &GROUND_NORMAL_TEXTURE },
        { "GROUND_SNOW_TEXTURE", &GROUND_SNOW_TEXTURE },
        { "GROUND_DESERT_TEXTURE", &GROUND_DESERT_TEXTURE },
        { "GROUND_ASH_TEXTURE", &GROUND_ASH_TEXTURE },

        { "NIGHT_TEMP", &NIGHT_TEMP },

        { "DEBUG_MODE", &DEBUG_MODE },

        { "DEATH", &DEATH },

        { "SHOW_BARS", &SHOW_BARS },

        { "MAX_AMB_TEMP_CHANGE_PER_SEC", &MAX_AMB_TEMP_CHANGE_PER_SEC },
        { "MAX_AMB_TEMP_CHANGE_FAST", &MAX_AMB_TEMP_CHANGE_FAST },

        { "SMOOTHING", &SMOOTHING },
    };

    static TierSpell g_heatSpells[] =
    {
        { nullptr, 0 },                    // None
        { "GPSunsHeight.esp", 0x0015B5 },  // Warm
        { "GPSunsHeight.esp", 0x0015B4 },  // Hot
        { "GPSunsHeight.esp", 0x0015B6 },  // Sweltering
        { "GPSunsHeight.esp", 0x0037F7 },  // Hyperthermia
    };

    static TierSpell g_coldSpells[] =
    {
        { nullptr, 0 },                    // None
        { "GPSunsHeight.esp", 0x0015BD },  // Chilly
        { "GPSunsHeight.esp", 0x0015BA },  // Cold
        { "GPSunsHeight.esp", 0x0015B8 },  // Freezing
        { "GPSunsHeight.esp", 0x0037F8 },  // Hypothermia
    };

    static TierSpell g_wetSpells[] =
    {
        { nullptr, 0 },                    // Dry
        { "GPSunsHeight.esp", 0x0015C6 },  // Damp
        { "GPSunsHeight.esp", 0x0015BF },  // Wet
        { "GPSunsHeight.esp", 0x0015C4 },  // Soaked
    };

    enum
    {
        kSlot_Head = 0x1 << TESBipedModelForm::kPart_Head,
        kSlot_Hair = 0x1 << TESBipedModelForm::kPart_Hair,
        kSlot_UpperBody = 0x1 << TESBipedModelForm::kPart_UpperBody,
        kSlot_LowerBody = 0x1 << TESBipedModelForm::kPart_LowerBody,
        kSlot_Hand = 0x1 << TESBipedModelForm::kPart_Hand,
        kSlot_Foot = 0x1 << TESBipedModelForm::kPart_Foot,
        kSlot_RightRing = 0x1 << TESBipedModelForm::kPart_RightRing,
        kSlot_LeftRing = 0x1 << TESBipedModelForm::kPart_LeftRing,
        kSlot_Amulet = 0x1 << TESBipedModelForm::kPart_Amulet,
        kSlot_Weapon = 0x1 << TESBipedModelForm::kPart_Weapon,
        kSlot_BackWeapon = 0x1 << TESBipedModelForm::kPart_BackWeapon,
        kSlot_SideWeapon = 0x1 << TESBipedModelForm::kPart_SideWeapon,
        kSlot_Quiver = 0x1 << TESBipedModelForm::kPart_Quiver,
        kSlot_Shield = 0x1 << TESBipedModelForm::kPart_Shield,
        kSlot_Torch = 0x1 << TESBipedModelForm::kPart_Torch,
        kSlot_Tail = 0x1 << TESBipedModelForm::kPart_Tail,
        kSlot_UpperLower = kSlot_UpperBody | kSlot_LowerBody,
        kSlot_UpperLowerFoot = kSlot_UpperLower | kSlot_Foot,
        kSlot_UpperLowerHandFoot = kSlot_UpperLowerFoot | kSlot_Hand,
        kSlot_UpperLowerHand = kSlot_UpperLower | kSlot_Hand,
        kSlot_BothRings = kSlot_RightRing | kSlot_LeftRing,
        kSlot_UpperHand = kSlot_UpperBody | kSlot_Hand,

        kSlot_None = 0,
    };

    float Manager::GetSlotWeight(UInt32 slot)
    {
        switch (0x1 << slot)
        {
        case kSlot_UpperBody: return SLOT_CHEST;
        case kSlot_LowerBody:  return SLOT_LEGS;
        case kSlot_Head:  return SLOT_HEAD;
        case kSlot_Foot:  return SLOT_FEET;
        case kSlot_Hand: return SLOT_HANDS;
        case kSlot_LeftRing:  return SLOT_RING;
        case kSlot_RightRing:  return SLOT_RING;
        case kSlot_Amulet:return SLOT_AMULET;
        default: return 0.0f;
        }
    }

    TESForm* Manager::ResolveForm(const char* modName, UInt32 localID)
    {
        UInt8 modIndex = ModTable::Get().GetModIndex(modName);
        if (modIndex == 0xFF)
            return nullptr;

        UInt32 fullID = (modIndex << 24) | localID;
        return LookupFormByID(fullID);
    }

    static UInt32 GetItemSlot(TESForm* type)
    {
        if (type)
        {
            TESBipedModelForm* bip = (TESBipedModelForm*)Oblivion_DynamicCast(type, 0, RTTI_TESForm, RTTI_TESBipedModelForm, 0);
            if (bip)
            {
                return bip->GetSlot();
            }
        }
        return kSlot_None;
    }

    float Manager::EstimateWarmthFromArmor(TESForm* eq)
    {
        float warmth = 0.0f;


        TESObjectARMO* armor = OBLIVION_CAST(eq, TESForm, TESObjectARMO);

        if (armor)
        {
            warmth = sqrt(armor->armorRating) * ARMOR_RATING_MULT;
        }
        else
        {
            TESObjectCLOT* clothing = OBLIVION_CAST(eq, TESForm, TESObjectCLOT);
            if (clothing)
            {
                warmth = clothing->weight.weight * CLOTHING_WEIGHT_MULT;
            }
        }

        return warmth;
    }

    float Manager::EstimateCoverageFromArmor(TESForm* eq)
    {
        float coverage = 0.0f;


        TESObjectARMO* armor = OBLIVION_CAST(eq, TESForm, TESObjectARMO);

        if (armor)
        {
            if (armor->IsHeavyArmor())
                coverage = HEAVY_ARMOR_COVERAGE;
            else if (!armor->IsHeavyArmor())
                coverage = LIGHT_ARMOR_COVERAGE;
        }
        else
        {
            TESObjectCLOT* clothing = OBLIVION_CAST(eq, TESForm, TESObjectCLOT);
            if (clothing)
            {
                coverage = CLOTHING_COVERAGE;
            }
        }

        return coverage;
    }

    void Manager::CalculateEquipment()
    {
        tempData.clothingWarmth = 0.0f;
        tempData.clothingCoverage = 0.0f;

        Actor* player = *g_thePlayer;
        if (!player || !player->process)
            return;

        EquippedItemsList worn = player->GetEquippedItems();
        for (int i = 0; i < worn.size(); i++)
        {

            TESForm* eq = worn.at(i);

            if (!eq)
                continue;

            float slotWeight = GetSlotWeight(GetItemSlot(eq));
            float warmth = 0.0f;
            float coverage = slotWeight;

            // Check override
            auto it = g_itemOverrides.find(eq->refID);
            if (it != g_itemOverrides.end())
            {
                warmth = it->second.warmth;
                coverage *= it->second.coverage;
            }
            else
            {
                warmth = EstimateWarmthFromArmor(eq);
                coverage *= EstimateCoverageFromArmor(eq);
            }
            
            tempData.clothingWarmth += warmth * slotWeight;
            tempData.clothingCoverage += coverage;
        }
    }

    void Manager::ResolveWeatherOverrides()
    {
        for (const auto& entry : g_weatherOverrideEntries)
        {

            TESForm* form = ResolveForm(entry.key.modName.c_str(), entry.key.localID);
            if (!form)
                continue;

            TESWeather* weather = OBLIVION_CAST(form, TESForm, TESWeather);
            if (!weather)
                continue;


            g_weatherOverrides[weather->refID] = entry.data;
        }
    }

    void Manager::ResolveRegionOverrides()
    {
        for (const auto& entry : g_regionOverrideEntries)
        {
            TESForm* form = ResolveForm(entry.key.modName.c_str(), entry.key.localID);
            if (!form)
                continue;

            TESRegion* region = OBLIVION_CAST(form, TESForm, TESRegion);
            if (!region)
                continue;

            g_regionOverrides[region->refID] = entry.data;
        }
    }

    void Manager::ResolveWorldspaceOverrides()
    {
        for (const auto& entry : g_worldOverrideEntries)
        {
            TESForm* form = ResolveForm(entry.key.modName.c_str(), entry.key.localID);
            if (!form)
                continue;

            TESWorldSpace* world = OBLIVION_CAST(form, TESForm, TESWorldSpace);
            if (!world)
                continue;

            g_worldOverrides[world->refID] = entry.data;
        }
    }

    std::vector<TESObjectREFR*> Manager::DetectNearbyFire()
    {
        std::vector<TESObjectREFR*> fires;

        if (!g_thePlayer || !(*g_thePlayer))
            return fires;

        PlayerCharacter* player = *g_thePlayer;
        float playerPosX = player->posX;
        float playerPosY = player->posY;
        const float MAX_FIRE_DIST_SQ = MAX_FIRE_DIST * MAX_FIRE_DIST;

        auto& refList = player->parentCell->objectList;
        CellListVisitor visitor = (&refList);

        for (int i = 0; i < visitor.Count(); ++i)
        {
            TESObjectREFR* ref = visitor.GetNthInfo(i);
            if (!ref)
                continue;

            TESForm* base = ref->baseForm;
            if (!base)
                continue;

            if (base->typeID != kFormType_Light &&
                base->typeID != kFormType_Activator &&
                base->typeID != kFormType_Stat)
                continue;

            // Only include objects with fire/flame/lava in editor ID
            try
            {
                std::string editorID = base->GetEditorName();
                std::transform(editorID.begin(), editorID.end(), editorID.begin(), ::tolower);
                if (editorID.find("fire") == std::string::npos &&
                    editorID.find("flame") == std::string::npos &&
                    editorID.find("lava") == std::string::npos)
                {
                    continue;
                }
            }
            catch (const std::exception&)
            {
                
            }

            float diffX = ref->posX - playerPosX;
            float diffY = ref->posY - playerPosY;
            float distSq = diffX * diffX + diffY * diffY;

            if (distSq > MAX_FIRE_DIST_SQ)
                continue;

            fires.push_back(ref);
        }

        return fires;
    }

    enum class GroundType
    {
        Normal,
        Snow,
        Desert,
        Ash
    };

    GroundType ClassifyLandTexture(TESLandTexture* tex)
    {
        if (!tex)
            return GroundType::Normal;

        const char* path = tex->texture.ddsPath.m_data;

        std::string p(path);
        std::transform(p.begin(), p.end(), p.begin(), ::tolower);

        if (p.find("snow") != std::string::npos ||
            p.find("frozen") != std::string::npos ||
            p.find("ice") != std::string::npos)
            return GroundType::Snow;

        if (p.find("sand") != std::string::npos ||
            p.find("desert") != std::string::npos)
            return GroundType::Desert;

        if (p.find("ash") != std::string::npos ||
            p.find("oblivion") != std::string::npos)
            return GroundType::Ash;

        return GroundType::Normal;
    }


    void Manager::CalculateAmbientTemp(TESObjectCELL* cell)
    {
        tempData.ambientTargetTemp = AMBIENT_TEMP_BASE;

        if (cell->IsInterior())
        {
            tempData.isInside = true;
            tempData.isRaining = false;
            tempData.isSnowing = false;
        }
        else
        {
            tempData.isInside = false;

            // Region
            ExtraRegionList* regionList = dynamic_cast<ExtraRegionList*>(cell->extraData.GetByType(kExtraData_RegionList));
            if (regionList && regionList->regionList)
            {
                TESRegionList* regions = regionList->regionList;
                TESRegionList::Entry* regionPtr = &(regions->regionList);

                while (regionPtr != NULL)
                {
                    UInt32 regionID = regionPtr->region->refID;

                    auto it = g_regionOverrides.find(regionID);
                    if (it != g_regionOverrides.end())
                    {
                        const Override& o = it->second;
                        tempData.ambientTargetTemp += o.tempOffset;
                    }

                    regionPtr = regionPtr->next;
                }
            }

            // Worldspace

            TESWorldSpace* world = cell->worldSpace;
            if (world)
            {
                auto it = g_worldOverrides.find(world->refID);
                if (it != g_worldOverrides.end())
                {
                    const Override& o = it->second;
                    tempData.ambientTargetTemp += o.tempOffset;
                }
            }


            // Altitude

            float z = (*g_thePlayer)->posZ;

            float altitudePenalty = z / ALTITUDE_PENALTY;

            if (!(*g_thePlayer)->parentCell || (*g_thePlayer)->parentCell->IsInterior())
                altitudePenalty = 0.0f;

            tempData.ambientTargetTemp -= altitudePenalty;

            // Weather

            TESWeather* weather = Sky::GetSingleton()->firstWeather;

            if (weather)
            {
                tempData.weatherID = weather->refID;

                auto it = g_weatherOverrides.find(weather->refID);
                if (it != g_weatherOverrides.end())
                {
                    const Override& o = it->second;
                    tempData.ambientTargetTemp += o.tempOffset;
                }
                else
                {
                    switch (weather->precipType)
                    {
                    case TESWeather::kType_Snow:
                        tempData.ambientTargetTemp += WEATHER_SNOW_TEMP;
                        tempData.isSnowing = true;
                        tempData.isRaining = false;
                        break;

                    case TESWeather::kType_Rainy:
                        tempData.ambientTargetTemp += WEATHER_RAIN_TEMP;
                        tempData.isRaining = true;
                        tempData.isSnowing = false;
                        break;

                    case TESWeather::kType_Cloudy:
                        tempData.ambientTargetTemp += WEATHER_CLOUDY_TEMP;
                        tempData.isRaining = false;
                        tempData.isSnowing = false;
                        break;

                    case TESWeather::kType_Pleasant:
                        tempData.ambientTargetTemp += WEATHER_PLEASANT_TEMP;
                        tempData.isRaining = false;
                        tempData.isSnowing = false;
                        break;
                    default:
                        tempData.ambientTargetTemp += WEATHER_DEFAULT_TEMP;
                        tempData.isRaining = false;
                        tempData.isSnowing = false;
                        break;
                    }
                }
                

                TESLandTexture* texture = GetLandTexture();

                if (texture)
                {
                    GroundType groundType = ClassifyLandTexture(texture);
                    switch (groundType)
                    {
                    case SunsHeight::GroundType::Normal:
                        tempData.ambientTargetTemp += GROUND_NORMAL_TEXTURE;
                        break;
                    case SunsHeight::GroundType::Snow:
                        tempData.ambientTargetTemp += GROUND_SNOW_TEXTURE;
                        break;
                    case SunsHeight::GroundType::Desert:
                        tempData.ambientTargetTemp += GROUND_DESERT_TEXTURE;
                        break;
                    case SunsHeight::GroundType::Ash:
                        tempData.ambientTargetTemp += GROUND_ASH_TEXTURE;
                        break;
                    default:
                        break;
                    }

                }
            }
            
        }
    }

    constexpr float CELL_SIZE = 4096.0f;
    constexpr int   LAND_QUADS = 32;
    constexpr float QUAD_SIZE = CELL_SIZE / LAND_QUADS;

    TESLandTexture* Manager::GetLandTexture()
    {
        TESObjectCELL* cell = (*g_thePlayer)->parentCell;
        if (!cell || cell->IsInterior())
            return nullptr;

        TESObjectLAND* land = cell->land;
        if (!land)
            return nullptr;

        TESLandTexture* landTex = land->data->textures[0];
        return landTex;
    }

    void Manager::UpdateExposureLevels()
    {
        float t = tempData.bodyTemp;

        // Reset
        tempData.heatLevel = HeatLevel::None;
        tempData.coldLevel = ColdLevel::None;
        tempData.wetLevel = WetLevel::Dry;


        if (t >= HOT_4 - 0.5f)      tempData.heatLevel = HeatLevel::Hyperthermia;
        else if (t >= HOT_3)      tempData.heatLevel = HeatLevel::Sweltering;
        else if (t >= HOT_2) tempData.heatLevel = HeatLevel::Hot;
        else if (t >= HOT_1) tempData.heatLevel = HeatLevel::Warm;

        if (t <= COLD_4 - 0.5f)      tempData.coldLevel = ColdLevel::Hypothermia;
        else if (t <= COLD_3)      tempData.coldLevel = ColdLevel::Freezing;
        else if (t <= COLD_2) tempData.coldLevel = ColdLevel::Cold;
        else if (t <= COLD_1) tempData.coldLevel = ColdLevel::Chilly;

        float w = tempData.wetness;

        if (w >= WET_3)      tempData.wetLevel = WetLevel::Soaked;
        else if (w >= WET_2) tempData.wetLevel = WetLevel::Wet;
        else if (w >= WET_1) tempData.wetLevel = WetLevel::Damp;
        else                 tempData.wetLevel = WetLevel::Dry;

    }

    bool AddSpellRuntime(Actor* actor, SpellItem* spell, int tier)
    {
        // Actor::AddSpell @ vtbl index 0xB7 (Oblivion 1.2.0416)
        void** vtbl = *(void***)actor;
        using AddSpellFn = bool(__thiscall*)(Actor*, SpellItem*);
        AddSpellFn addSpell =
            reinterpret_cast<AddSpellFn>(vtbl[0xB7]); // AddSpell
        std::string notif = "I am " + std::string((spell->GetFullName())->name.m_data) + ".";
        if (tier >= 4)
        {
            notif = "I have " + std::string((spell->GetFullName())->name.m_data) + ".";
        }
        QueueUIMessage(notif.c_str(), 0, 1, 5);
        return addSpell(actor, spell);
    }

    bool RemoveSpellRuntime(Actor* actor, SpellItem* spell)
    {
        if (!actor || !spell)
            return false;

        // Actor::RemoveSpell @ vtbl index 0xB8 (Oblivion 1.2.0416)
        void** vtbl = *(void***)actor;
        using RemoveSpellFn = bool(__thiscall*)(Actor*, SpellItem*);
        RemoveSpellFn addSpell =
            reinterpret_cast<RemoveSpellFn>(vtbl[0xB8]); // AddSpell
        return addSpell(actor, spell);
    }

    void Manager::ApplyTierSpell(
        Actor* player,
        int newTier,
        int& lastTier,
        SpellItem** spellTable)
    {
        if (newTier == lastTier)
            return;

        if (newTier == 4 && DEATH <= 0.0f)
            return;

        // Remove old
        if (lastTier >= 0 && spellTable[lastTier])
        {
            RemoveSpellRuntime(player, spellTable[lastTier]);
        }

        // Apply new
        if (newTier >= 0 && spellTable[newTier])
        {
            AddSpellRuntime(player, spellTable[newTier], newTier);
        }
        
        if (lastTier >= 0 && newTier == 0 && tempData.bodyTemp > COLD_1 && tempData.bodyTemp < HOT_1)
        {
            QueueUIMessage("I am content.", 0, 1, 5);
        }

        lastTier = newTier;
    }

    void Manager::ResolveTierSpells()
    {
        auto resolveTable = [this](TierSpell* src, SpellItem** dst, int count) {
            for (int i = 0; i < count; i++)
            {
                if (!src[i].modName)
                {
                    dst[i] = nullptr;
                    continue;
                }

                TESForm* form =
                    ResolveForm(src[i].modName, src[i].localID);

                dst[i] = OBLIVION_CAST(form, TESForm, SpellItem);
            }
            };

        resolveTable(g_heatSpells, g_heatSpellForms, 5);
        resolveTable(g_coldSpells, g_coldSpellForms, 5);
        resolveTable(g_wetSpells, g_wetSpellForms, 5);
    }

    void Manager::ResolveGlobals()
    {
        g_bodyTempGlobal = OBLIVION_CAST(ResolveForm("GPSunsHeight.esp", 0x3ECC), TESForm, TESGlobal);
        g_ambientTempGlobal = OBLIVION_CAST(ResolveForm("GPSunsHeight.esp", 0x3ECD), TESForm, TESGlobal);
        g_coverageGlobal = OBLIVION_CAST(ResolveForm("GPSunsHeight.esp", 0x3ECE), TESForm, TESGlobal);
        g_warmthGlobal = OBLIVION_CAST(ResolveForm("GPSunsHeight.esp", 0x3ECF), TESForm, TESGlobal);
        g_wetnessGlobal = OBLIVION_CAST(ResolveForm("GPSunsHeight.esp", 0x3ED0), TESForm, TESGlobal);
        g_fireHeatGlobal = OBLIVION_CAST(ResolveForm("GPSunsHeight.esp", 0x3ED1), TESForm, TESGlobal);
        g_nearFireGlobal = OBLIVION_CAST(ResolveForm("GPSunsHeight.esp", 0x3ED2), TESForm, TESGlobal);
        g_showBarsGlobal = OBLIVION_CAST(ResolveForm("GPSunsHeight.esp", 0x5352), TESForm, TESGlobal);
    }

    void Manager::ApplyTemperatureTierSpells()
    {
        Actor* player = *g_thePlayer;
        if (!player)
            return;

        ApplyTierSpell(
            player,
            (int)tempData.heatLevel,
            (int&)tempData.lastHeatLevel,
            g_heatSpellForms);

        ApplyTierSpell(
            player,
            (int)tempData.coldLevel,
            (int&)tempData.lastColdLevel,
            g_coldSpellForms);

        ApplyTierSpell(
            player,
            (int)tempData.wetLevel,
            (int&)tempData.lastWetLevel,
            g_wetSpellForms);
    }

    void DebugPrintTemperatureState(const TemperatureData& tempData)
    {
        if (DEBUG_MODE == 0.0f)
            return;


        _MESSAGE(
            "[Temperature DEBUG] "
            "Final=%.2f | Ambient=%.2f | Wetness=%.2f | Wet Change=%.2f | Clothing Warmth=%.2f| Clothing Coverage=%.2f| Fire Heat=%.2f | Near Fire = %d",
            tempData.bodyTemp,
            tempData.ambientTemp,
            tempData.wetness,
            WET_CHANGE,
            tempData.clothingWarmth,
            tempData.clothingCoverage,
            tempData.fireHeat,
            tempData.nearFire
        );
    }

    void Manager::WriteToGlobals()
    {
        if (g_bodyTempGlobal)
        {
           g_bodyTempGlobal->data = tempData.bodyTemp;
        }

        if (g_ambientTempGlobal)
        {
            g_ambientTempGlobal->data = tempData.ambientTemp;
        }

        if (g_coverageGlobal)
        {
            g_coverageGlobal->data = tempData.clothingCoverage;
        }

        if (g_warmthGlobal)
        {
            g_warmthGlobal->data = tempData.clothingWarmth;
        }

        if (g_wetnessGlobal)
        {
            g_wetnessGlobal->data = tempData.wetness;
        }

        if (g_fireHeatGlobal)
        {
            g_fireHeatGlobal->data = tempData.fireHeat;
        }

        if (g_nearFireGlobal)
        {
            g_nearFireGlobal->data = tempData.nearFire;
        }

        if (g_showBarsGlobal)
        {
            g_showBarsGlobal->data = SHOW_BARS;
        }

    }

    void Manager::UpdateAmbientTemp(bool didTimeJump, float deltaSeconds)
    {
        float smoothing = SMOOTHING * deltaSeconds;
        tempData.ambientTemp += (tempData.ambientTargetTemp - tempData.ambientTemp) * smoothing;

    }

    void Manager::UpdateTemperature()
    {
        if (!InterfaceManager::GetSingleton()->IsGameMode() && !Manager::realTimeMenusLoaded)
            return;

        if (!g_thePlayer || !(*g_thePlayer))
            return;

        PlayerCharacter* player = *g_thePlayer;
        TESObjectCELL* currentCell = player->parentCell;
        if (!currentCell) return;

        float now = TimeGlobals::Singleton()->GameHour() * 3600.0f; // seconds

        // Rescan fires only on cell change or every 5 seconds
        if (currentCell != cell || now - lastFireScanTime > 5.0f)
        {
            if (DEBUG_MODE)
                _MESSAGE("Fire scan performed");

            cell = currentCell;
            cachedFires = DetectNearbyFire();
            lastFireScanTime = now;
        }

        // Compute fire heat from cached fires every tick
        tempData.fireHeat = 0.0f;
        tempData.nearFire = !cachedFires.empty();
        for (TESObjectREFR* ref : cachedFires)
        {
            if (!ref) continue;

            float diffX = ref->posX - player->posX;
            float diffY = ref->posY - player->posY;
            float distSq = diffX * diffX + diffY * diffY;

            if (distSq > MAX_FIRE_DIST * MAX_FIRE_DIST)
                continue;

            float dist = sqrtf(distSq);
            float heatFactor = 1.0f;

            TESForm* base = ref->baseForm;
            if (base && base->typeID == kFormType_Light)
            {
                try {
                    std::string editorID = base->GetEditorName();
                    std::transform(editorID.begin(), editorID.end(), editorID.begin(), ::tolower);

                    if (editorID.find("small") != std::string::npos) heatFactor = SMALL_FIRE_FACTOR;
                    else if (editorID.find("medium") != std::string::npos) heatFactor = MEDIUM_FIRE_FACTOR;
                    else if (editorID.find("large") != std::string::npos) heatFactor = LARGE_FIRE_FACTOR;
                }
                catch (const std::runtime_error& e) {}
                
            }

            float heat = std::max(0.0f, 1.0f - (dist / MAX_FIRE_DIST));
            tempData.fireHeat += heat * HEAT_MULT * heatFactor;
            if (tempData.fireHeat >= 100.0f) break;
        }

        tempData.fireHeat = std::clamp(tempData.fireHeat, 0.0f, 100.0f);

        // --- Calculate environmental factors ---
        CalculateAmbientTemp(cell);
        CalculateEquipment();

        bool isSwimming = ((*g_thePlayer)->process->GetMovementFlags() & BaseProcess::kMovementFlag_Swimming) != 0;
        if (isSwimming) { tempData.wetness = 100.0f; }

        // --- Determine wetness change ---
        WET_CHANGE = 0.0f;
        if (!cell->IsInterior())
        {
            if (tempData.isRaining)
                WET_CHANGE += WET_GAIN_RAIN;
            else if (tempData.isSnowing)
                WET_CHANGE += WET_GAIN_SNOW;
        }
        else
        {
            WET_CHANGE -= WET_DRY_IN;
        }

        if (!tempData.isRaining && !tempData.isSnowing)
            WET_CHANGE -= WET_DRY_OUT;

        if (tempData.nearFire)
            WET_CHANGE -= WET_DRY_FIRE;


        // --- Time delta ---
        float currentHour = TimeGlobals::Singleton()->GameHour();
        if (tempData.lastGameHour < 0.0f)
            tempData.lastGameHour = currentHour;

        float deltaHours = currentHour - tempData.lastGameHour;
        if (deltaHours < 0.0f) deltaHours += 24.0f;
        tempData.lastGameHour = currentHour;

        float deltaSeconds = deltaHours * 3600.0f;
        bool didTimeJump = deltaHours > 0.25f;

        float ambientTarget = tempData.ambientTargetTemp;
        float nightFactor = 0.0f;

        if (currentHour >= 20.0f)
            nightFactor = (currentHour - 20.0f) / 4.0f;
        else if (currentHour < 6.0f)
            nightFactor = (6.0f - currentHour) / 6.0f;

        nightFactor = std::clamp(nightFactor, 0.0f, 1.0f);
        ambientTarget += NIGHT_TEMP * nightFactor;

        tempData.ambientTemp = tempData.ambientTemp + (ambientTarget - tempData.ambientTemp) * SMOOTHING;


        // --- Smooth wetness ---
        tempData.wetness += WET_CHANGE * deltaSeconds / 60.0f;
        tempData.wetness = std::clamp(tempData.wetness, 0.0f, 100.0f);

        float targetTemp = tempData.ambientTemp;

        float insulation = tempData.clothingWarmth * 0.01f * tempData.clothingCoverage;

        insulation = std::clamp(insulation, 0.0f, 0.85f);

        insulation *= (1.0f - tempData.wetness / 100.0f);

        if (tempData.nearFire && (!tempData.isInside || tempData.bodyTemp < 50))
        {
            float fireEffect =
                tempData.fireHeat *
                (0.5f + 0.5f * (1.0f - insulation)); // insulation partially blocks fire

            targetTemp += fireEffect;
        }

        float heatTrap = std::max(0.0f, tempData.ambientTemp - 50.0f);
        targetTemp += heatTrap * insulation;

        float tempDiff = targetTemp - tempData.bodyTemp;

        float rateMultiplier = 1.0f;

        if (tempDiff < 0.0f)
            rateMultiplier += (tempData.wetness / 100.0f) * 1.5f;
        else
            rateMultiplier *= (1.0f - tempData.wetness / 100.0f);

        
        float responsiveness =
            RATE_PER_SECOND *
            (1.0f - insulation) *
            rateMultiplier;

        float maxStep = MAX_AMB_TEMP_CHANGE_PER_SEC * deltaSeconds;
        float tempStep = std::clamp(tempDiff * responsiveness, -maxStep, maxStep);

        tempData.bodyTemp += tempStep;
        tempData.bodyTemp = std::clamp(tempData.bodyTemp, 0.0f, 100.0f);

        // --- Update exposure and spells ---
        UpdateExposureLevels();
        ApplyTemperatureTierSpells();

        // --- Write to globals ---
        WriteToGlobals();

        DebugPrintTemperatureState(tempData);
    }


    static inline void Trim(std::string& s)
    {
        s.erase(0, s.find_first_not_of(" \t\r\n"));
        s.erase(s.find_last_not_of(" \t\r\n") + 1);
    }

    void LoadConfig()
    {
        std::ifstream file("Data\\OBSE\\Plugins\\SunsHeight\\Config.ini");
        if (!file.is_open())
        {
            _MESSAGE("Config.ini not found");
            return;
        }

        std::string line;
        enum Section { NONE, WORLDS, REGIONS, WEATHER, CONFIG } section = NONE;

        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == ';')
                continue;

            if (line[0] == '[')
            {
                if (line == "[Worldspaces]") section = WORLDS;
                else if (line == "[Regions]") section = REGIONS;
                else if (line == "[Weathers]") section = WEATHER;
                else if (line == "[Config]") section = CONFIG;
                else section = NONE;
                continue;
            }

            std::stringstream ss(line);

            if (section == WORLDS)
            {
                OverrideKey key;
                Override worldOverride;
                OverrideEntry entry;
                std::string modName;
                std::string idStr;

                std::getline(ss, modName, ',');
                std::getline(ss, idStr, ',');
                ss >> worldOverride.tempOffset;
                
                UInt32 id = static_cast<UInt32>(std::stoul(idStr, nullptr, 16));

                key.modName = modName.c_str();
                key.localID = id;

                entry.key = key;
                entry.data = worldOverride;

                g_worldOverrideEntries.push_back(entry);
            }
            else if (section == REGIONS)
            {
                OverrideKey key;
                Override regionOverride;
                OverrideEntry entry;
                std::string modName;
                std::string idStr;

                std::getline(ss, modName, ',');
                std::getline(ss, idStr, ',');
                ss >> regionOverride.tempOffset;
                
                UInt32 id = static_cast<UInt32>(std::stoul(idStr, nullptr, 16));

                key.modName = modName.c_str();
                key.localID = id;

                entry.key = key;
                entry.data = regionOverride;

                g_regionOverrideEntries.push_back(entry);
            }
            else if (section == WEATHER)
            {
                OverrideKey key;
                Override weatherOverride;
                OverrideEntry entry;
                std::string modName;
                std::string idStr;

                std::getline(ss, modName, ',');
                std::getline(ss, idStr, ',');
                ss >> weatherOverride.tempOffset;

                UInt32 id = static_cast<UInt32>(std::stoul(idStr, nullptr, 16));

                key.modName = modName.c_str();
                key.localID = id;

                entry.key = key;
                entry.data = weatherOverride;

                g_weatherOverrideEntries.push_back(entry);
            }
            else if (section == CONFIG)
            {
                std::string key, value;

                std::getline(ss, key, ',');
                std::getline(ss, value);

                float f = std::stof(value);

                auto it = configMap.find(key);
                if (it != configMap.end())
                {
                    *it->second = f;
                }
            }
        }
    }

    void Manager::Initialize()
    {
        LoadConfig();
        ResolveWeatherOverrides();
        ResolveRegionOverrides();
        ResolveWorldspaceOverrides();
        ResolveTierSpells();
        ResolveGlobals();
        _MESSAGE("Sun's Height initialized");
    }
}
