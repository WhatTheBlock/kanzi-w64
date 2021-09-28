/*
Copyright 2011-2017 Frederic Langlet
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
you may obtain a copy of the License at

                http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <sstream>
#include "TextCodec.hpp"
#include "../Global.hpp"

using namespace kanzi;

// 1024 of the most common English words with at least 2 chars.
char TextCodec::DICT_EN_1024[] =
"TheBeAndOfInToWithItThatForYouHeHaveOnSaidSayAtButWeByHadTheyAsW\
ouldWhoOrCanMayDoThisWasIsMuchAnyFromNotSheWhatTheirWhichGetGive\
HasAreHimHerComeMyOurWereWillSomeBecauseThereThroughTellWhenWork\
ThemYetUpOwnOutIntoJustCouldOverOldThinkDayWayThanLikeOtherHowTh\
enItsPeopleTwoMoreTheseBeenNowWantFirstNewUseSeeTimeManManyThing\
MakeHereWellOnlyHisVeryAfterWithoutAnotherNoAllBelieveBeforeOffT\
houghSoAgainstWhileLastTooDownTodaySameBackTakeEachDifferentWher\
eBetweenThoseEvenSeenUnderAboutOneAlsoFactMustActuallyPreventExp\
ectContainConcernMeSchoolYearTipLabDueEverTowardGirlFirmGlassGas\
KeepWorldStillWentShouldSpendStageDoctorMightJobGoContinueEveryo\
neNeverAnswerFewMeanDifferenceTendNeedLeaveTryNiceHoldSomethingA\
skWarmLipCoverIssueHappenTurnLookSureDiscoverFightMadDirectionAg\
reeSomeoneFailRespectNoticeChoiceBeginThreeSystemLevelFeelMeetCo\
mpanyBoxShowPlayLiveLetterEggNumberOpenProblemFatHandMeasureQues\
tionCallRememberCertainPutNextChairStartRunRaiseGoalReallyHomeTe\
aCandidateMoneyBusinessYoungGoodCourtFindKnowKindHelpNightChildL\
otYourUsEyeYesWordBitVanMonthHalfLowMillionHighOrganizationRedGr\
eenBlueWhiteBlackYourselfEightBothLittleHouseLetDespiteProvideSe\
rviceHimselfFriendDescribeFatherDevelopmentAwayKillTripHourGameO\
ftenPlantPlaceEndAmongSinceStandDesignParticularSuddenlyMemberPa\
yLawBookSilenceAlmostIncludeAgainEitherToolFiveOnceLeastExplainI\
dentifyUntilSiteMinuteCoupleWeekMatterBringDetailInformationNoth\
ingAnythingEverythingAgoLeadSometimesUnderstandWhetherNatureToge\
therFollowParentStopIndeedDifficultPublicAlreadySpeakMaintainRem\
ainHearAllowMediaOfficeBenefitDoorHugPersonLaterDuringWarHistory\
ArgueWithinSetArticleStationMorningWalkEventWinChooseBehaviorSho\
otFireFoodTitleAroundAirTeacherGapSubjectEnoughProveAcrossAlthou\
ghHeadFootSecondBoyMainLieAbleCivilTableLoveProcessOfferStudentC\
onsiderAppearStudyBuyNearlyHumanEvidenceTextMethodIncludingSendR\
ealizeSenseBuildControlAudienceSeveralCutCollegeInterestSuccessS\
pecialRiskExperienceBehindBetterResultTreatSixRelationshipAnimal\
ImproveHairStayTopReducePerhapsLateWriterPickElseSignificantChan\
ceHotelGeneralRockRequireAlongFitThemselvesReportConditionReachT\
ruthEffortDecideRateEducationForceGardenDrugLeaderVoiceQuiteWhol\
eSeemMindFinallySirReturnFreeStoryRespondPushAccordingBrotherLea\
rnSonHopeDevelopFeelingReadCarryDiseaseRoadVariousBallCaseOperat\
ionCloseVisitReceiveBuildingValueResearchFullModelJoinSeasonTaxD\
irectorPositionPlayerSportErrorRecordRowDataPaperTheorySpaceEver\
yFormSupportActionOfficialWhoseIdeaHappyHeartBestTeamProjectHitB\
aseRepresentTownPullBusMapDryMomCatDadRoomSmileFieldImpactFundLa\
rgeDogHugePrepareEnvironmentalProduceHerselfTeachOilSuchSituatio\
nTieCostIndustrySkinStreetImageItselfPhonePriceWearMostSunSoonCl\
earPracticePieceWaitRecentImportantProductLeftWallSeriesNewsShar\
eMovieKidNorSimplyWifeOntoCatchMyselfFineComputerSongAttentionDr\
awFilmRepublicanSecurityScoreTestStockPositiveCauseCenturyWindow\
MemoryExistListenStraightCultureBillionFormerDecisionEnergyMoveS\
ummerWonderRelateAvailableLineLikelyOutsideShotShortCountryRoleA\
reaSingleRuleDaughterMarketIndicatePresentLandCampaignMaterialPo\
pulationEconomyMedicalHospitalChurchGroundThousandAuthorityInste\
adRecentlyFutureWrongInvolveLifeHeightIncreaseRightBankCulturalC\
ertainlyWestExecutiveBoardSeekLongOfficerStatementRestBayDealWor\
kerResourceThrowForwardPolicyScienceEyesBedItemWeaponFillPlanMil\
itaryGunHotHeatAddressColdFocusForeignTreatmentBloodUponCourseTh\
irdWatchAffectEarlyStoreThusSoundEverywhereBabyAdministrationMou\
thPageEnterProbablyPointSeatNaturalRaceFarChallengePassApplyMail\
UsuallyMixToughClearlyGrowFactorStateLocalGuyEastSaveSouthSceneM\
otherCareerQuicklyCentralFaceIceAboveBeyondPictureNetworkManagem\
entIndividualFanSizeSpeedBusySeriousOccurAddReadySignCollectionL\
istApproachChargeQualityPressureVoteNotePartRealWebCurrentDeterm\
ineTrueSadWhateverBreakWorryCupParticularlyAmountAbilityEatRecog\
nizeSitCharacterSomebodyLossDegreeEffectAttackStaffMiddleTelevis\
ionWhyLegalCapitalTradeElectionEverybodyDropMajorViewStandardBil\
lEmployeeDiscussionOpportunityAnalysisTenSuggestLawyerHusbandSec\
tionBecomeSkillSisterStyleCrimeProgramCompareCapMissBadSortTrain\
ingEasyNearRegionStrategyPurposePerformTechnologyEconomicBudgetE\
xampleCheckEnvironmentLegDarkTermRatherLaughGuessCarLowerHangPas\
tSocialForgetHundredRemoveManagerEnjoyExactlyDieFinalMaybeHealth\
FloorChangeLayPoorFunEstablishTrialSpringDinnerBigThankProtectAv\
oidImagineTonightStarArmFinishMusicOwnerCryArtPrivateOthersSimpl\
ePopularReflectEspeciallySmallLightMessageStepKeyPeaceProgressBa\
nSideGreatFixInterviewManageNationalFishLoseCameraDiscussEqualWe\
ightPerformanceSevenWaterProductionPersonalCellPowerEveningColor\
InsideBarUnitLessAdultWideRangeMentionDeepEdgeStrongHardTroubleN\
ecessarySafeCommonFearFamilySeaDreamConferenceReplyPropertyMeeti\
ngAlwaysStuffAgencyDeathGrowthSellSoldierActHeavyWetBagMarriageD\
eadSingRiseDecadeWhomFigurePoliceBodyMachineCategoryAheadFrontCa\
reOrderRealityPartnerYardBeatViolenceTotalDefenseWriteConsumerCe\
nterGroupThoughtModernTaskCoachReasonAgeFingerSpecificConnection\
WishResponsePrettyMovementCardLogPenSumTreeEntireCitizenThrougho\
utPetSimilarVictimNewspaperThreatClassShakeSourceAccountPainFall\
RichPossibleAcceptSolidTravelTalkSkyCreateNonePlentyPeriodDefine\
NormalRevealDrinkAuthorServeNameMomentAgentDocumentActivityAnywa\
yAfraidTypeActiveTrainInterestingRadioDangerGenerationLeafCopyMa\
tchClaimAnyoneSoftwarePartyDeviceCodeLanguageLinkHoweverConfirmC\
ommentCityAnywhereSomewhereDebateDriveHigherBeautifulOnlineWoman\
PriorityTraditionalFourFinancial";

DictEntry TextCodec::STATIC_DICTIONARY[1024] = {};
bool TextCodec::DELIMITER_CHARS[256] = {};
bool TextCodec::TEXT_CHARS[256] = {};
const bool TextCodec::INIT = TextCodec::init(TextCodec::DELIMITER_CHARS, TextCodec::TEXT_CHARS);
const int TextCodec::STATIC_DICT_WORDS = TextCodec::createDictionary(DICT_EN_1024, sizeof(DICT_EN_1024), STATIC_DICTIONARY, 1024, 0);

bool TextCodec::init(bool delims[256], bool text[256])
{
    for (int i = 0; i < 256; i++) {
        if ((i >= ' ') && (i <= '/')) // [ !"#$%&'()*+,-./]
            delims[i] = true;
        else if ((i >= ':') && (i <= '?')) // [:;<=>?]
            delims[i] = true;
        else {
            switch (i) {
            case '\n':
            case '\r':
            case '\t':
            case '_':
            case '|':
            case '{':
            case '}':
            case '[':
            case ']':
                delims[i] = true;
                break;
            default:
                delims[i] = false;
            }
        }

        text[i] = isUpperCase(byte(i)) | isLowerCase(byte(i));
    }

    return true;
}

// Create dictionary from array of words
int TextCodec::createDictionary(char words[], int dictSize, DictEntry dict[], int maxWords, int startWord)
{
    int delimAnchor = 0;
    int32 h = HASH1;
    int nbWords = startWord;

    for (int i = 0; ((i < dictSize) && (nbWords < maxWords)); i++) {
        const byte b = byte(words[i]);

        if (isText(b) == false)
            continue;

        if (isUpperCase(b)) {
            if (i > delimAnchor) {
                dict[nbWords] = DictEntry(reinterpret_cast<byte*>(&words[delimAnchor]), h, nbWords, i - delimAnchor);
                nbWords++;
                delimAnchor = i;
                h = HASH1;
            }

            words[i] ^= 0x20;
        }

        h = h * HASH1 ^ int32(words[i]) * HASH2;
    }

    if (nbWords < maxWords) {
        dict[nbWords] = DictEntry(reinterpret_cast<byte*>(&words[delimAnchor]), h, nbWords, dictSize - 1 - delimAnchor);
        nbWords++;
    }

    return nbWords;
}

// return 8-bit status (see MASK flags constants)
byte TextCodec::computeStats(const byte block[], int count, int32 freqs0[])
{
    int32 freqs[256][256] = { { 0 } };
    int32 f0[256] = { 0 };
    int32 f1[256] = { 0 };
    int32 f3[256] = { 0 };
    int32 f2[256] = { 0 };
    const uint8* data = reinterpret_cast<const uint8*>(&block[0]);
    uint8 prv = 0;
    const int count4 = count & -4;

    // Unroll loop
    for (int i = 0; i < count4; i += 4) {
        const uint8 cur0 = data[i];
        const uint8 cur1 = data[i + 1];
        const uint8 cur2 = data[i + 2];
        const uint8 cur3 = data[i + 3];
        f0[cur0]++;
        f1[cur1]++;
        f2[cur2]++;
        f3[cur3]++;
        freqs[prv][cur0]++;
        freqs[cur0][cur1]++;
        freqs[cur1][cur2]++;
        freqs[cur2][cur3]++;
        prv = cur3;
    }

    for (int i = count4; i < count; i++) {
        freqs0[data[i]]++;
        freqs[prv][data[i]]++;
        prv = data[i];
    }

    for (int i = 0; i < 256; i++) {
        freqs0[i] += (f0[i] + f1[i] + f2[i] + f3[i]);
    }

    int nbTextChars = 0;

    for (int i = 32; i < 128; i++) {
        if (isText(byte(i)) == true)
            nbTextChars += freqs0[i];
    }

    // Not text (crude threshold)
    if (2 * nbTextChars < count)
        return TextCodec::MASK_NOT_TEXT;

    int nbBinChars = 0;

    for (int i = 128; i < 256; i++)
        nbBinChars += freqs0[i];

    // Not text (crude threshold)
    if ((4 * nbBinChars > count) || (16 * freqs0[32] < count))
        return TextCodec::MASK_NOT_TEXT;

    byte res = byte(0);

    if (nbBinChars == 0)
        res |= TextCodec::MASK_FULL_ASCII;
    else if (nbBinChars <= count / 100)
        res |= TextCodec::MASK_ALMOST_FULL_ASCII;

    if (nbBinChars <= count - count / 10) {
        // Check if likely XML/HTML
        // Another crude test: check that the frequencies of < and > are similar
        // and 'high enough'. Also check it is worth to attempt replacing ampersand sequences.
        // Getting this flag wrong results in a very small compression speed degradation.
        const int32 f1 = freqs0[60]; // '<'
        const int32 f2 = freqs0[62]; // '>'
        const int32 f3 = freqs[38][97] + freqs[38][103] + freqs[38][108] + freqs[38][113]; // '&a', '&g', '&l', '&q'
        const int32 minFreq = (((count - nbBinChars) >> 9) < 2) ? 2 : (count - nbBinChars) >> 9;

        if ((f1 >= minFreq) && (f2 >= minFreq) && (f3 > 0)) {
            if (f1 < f2) {
                if (f1 >= (f2 - f2 / 100))
                    res |= TextCodec::MASK_XML_HTML;
            }
            else if (f2 < f1) {
                if (f2 >= (f1 - f1 / 100))
                    res |= TextCodec::MASK_XML_HTML;
            }
            else
                res |= TextCodec::MASK_XML_HTML;
        }
    }

    // Check CR+LF matches
    const int cr = int(CR);
    const int lf = int(LF);

    if ((freqs0[cr] != 0) && (freqs0[cr] == freqs0[lf])) {
        res |= TextCodec::MASK_CRLF;

        for (int i = 0; i < 256; i++) {
            if ((i != lf) && (freqs[cr][i]) != 0) {
                res &= ~TextCodec::MASK_CRLF;
                break;
            }
        }
    }

    return res;
}

TextCodec::TextCodec()
{
    _delegate = new TextCodec1();
}

TextCodec::TextCodec(Context& ctx)
{
    int encodingType = ctx.getInt("textcodec", 1);
    _delegate = (encodingType == 1) ? reinterpret_cast<Function<byte>*>(new TextCodec1(ctx)) :
        reinterpret_cast<Function<byte>*>(new TextCodec2(ctx));
}

bool TextCodec::forward(SliceArray<byte>& input, SliceArray<byte>& output, int count) THROW
{
    if (count == 0)
        return true;

    if (!SliceArray<byte>::isValid(input))
        throw invalid_argument("Invalid input block");

    if (!SliceArray<byte>::isValid(output))
        throw invalid_argument("Invalid output block");

    if (input._array == output._array)
        return false;

    if (count > MAX_BLOCK_SIZE) {
        // Not a recoverable error: instead of silently fail the transform,
        // issue a fatal error.
        stringstream ss;
        ss << "The max text transform block size is " << MAX_BLOCK_SIZE << ", got " << count;
        throw invalid_argument(ss.str());
    }

    return _delegate->forward(input, output, count);
}

bool TextCodec::inverse(SliceArray<byte>& input, SliceArray<byte>& output, int count) THROW
{
    if (count == 0)
        return true;

    if (!SliceArray<byte>::isValid(input))
        throw invalid_argument("Invalid input block");

    if (!SliceArray<byte>::isValid(output))
        throw invalid_argument("Invalid output block");

    if (input._array == output._array)
        return false;

    if (count > MAX_BLOCK_SIZE) {
        // Not a recoverable error: instead of silently fail the transform,
        // issue a fatal error.
        stringstream ss;
        ss << "The max text transform block size is " << MAX_BLOCK_SIZE << ", got " << count;
        throw invalid_argument(ss.str());
    }

    return _delegate->inverse(input, output, count);
}

TextCodec1::TextCodec1()
{
    _logHashSize = TextCodec::LOG_HASHES_SIZE;
    _dictSize = TextCodec::THRESHOLD2 * 4;
    _dictMap = nullptr;
    _dictList = nullptr;
    _hashMask = (1 << _logHashSize) - 1;
    _staticDictSize = TextCodec::STATIC_DICT_WORDS;
    _isCRLF = false;
}

TextCodec1::TextCodec1(Context& ctx)
{
    // Actual block size
    int blockSize = 0;
    int log = 13;
    int dSize = 1 << 12;

    if (ctx.has("blockSize")) {
        blockSize = ctx.getInt("blockSize");

        if (blockSize >= 8)
            log = max(min(Global::log2(blockSize / 8), 26), 13);

        // Select an appropriate initial dictionary size
        dSize = 1 << max(min(log - 4, 18), 12);
    }

    uint extraMem = 0;

    if (ctx.has("extra")) {
        string strExtra = ctx.getString("extra");
        extraMem = (strExtra == STR_TRUE) ? 1 : 0;
    }

    _logHashSize = log + extraMem;
    _dictSize = dSize;
    _dictMap = nullptr;
    _dictList = nullptr;
    _hashMask = (1 << _logHashSize) - 1;
    _staticDictSize = TextCodec::STATIC_DICT_WORDS;
    _isCRLF = false;
}

void TextCodec1::reset()
{
    const int mapSize = 1 << _logHashSize;

    if (_dictMap == nullptr)
        _dictMap = new DictEntry * [mapSize];

    for (int i = 0; i < mapSize; i++)
        _dictMap[i] = nullptr;

    if (_dictList == nullptr) {
        _dictList = new DictEntry[_dictSize];
        const int nbEntries = min(TextCodec::STATIC_DICT_WORDS, _dictSize);
        memcpy(static_cast<void*>(&_dictList[0]), &TextCodec::STATIC_DICTIONARY[0], nbEntries * sizeof(DictEntry));

        // Add special entries at start of map
        _escapes[0] = TextCodec::ESCAPE_TOKEN2;
        _escapes[1] = TextCodec::ESCAPE_TOKEN1;
        const int nbWords = TextCodec::STATIC_DICT_WORDS;
        _dictList[nbWords] = DictEntry(&_escapes[0], 0, nbWords, 1);
        _dictList[nbWords + 1] = DictEntry(&_escapes[1], 0, nbWords + 1, 1);
        _staticDictSize = nbWords + 2;
    }

    for (int i = 0; i < _staticDictSize; i++)
        _dictMap[_dictList[i]._hash & _hashMask] = &_dictList[i];

    // Pre-allocate all dictionary entries
    for (int i = _staticDictSize; i < _dictSize; i++)
        _dictList[i] = DictEntry(nullptr, 0, i, 0);
}

bool TextCodec1::forward(SliceArray<byte>& input, SliceArray<byte>& output, int count)
{
    if (output._length - output._index < getMaxEncodedLength(count))
        return false;

    byte* src = &input._array[input._index];
    byte* dst = &output._array[output._index];
    int srcIdx = 0;
    int dstIdx = 0;

    int32 freqs[256] = { 0 };
    byte mode = TextCodec::computeStats(&src[srcIdx], count, freqs);

    // Not text ?
    if ((mode & TextCodec::MASK_NOT_TEXT) != byte(0))
        return false;

    reset();
    const int srcEnd = count;
    const int dstEnd = getMaxEncodedLength(count);
    const int dstEnd4 = dstEnd - 4;
    int emitAnchor = 0; // never less than 0
    int words = _staticDictSize;

    // DOS encoded end of line (CR+LF) ?
    _isCRLF = int(mode & TextCodec::MASK_CRLF) != 0;
    dst[dstIdx++] = mode;
    bool res = true;

    while ((srcIdx < srcEnd) && (src[srcIdx] == TextCodec::SP)) {
        dst[dstIdx++] = TextCodec::SP;
        srcIdx++;
        emitAnchor++;
    }

    int delimAnchor = TextCodec::isText(src[srcIdx]) ? srcIdx - 1 : srcIdx; // previous delimiter

    while (srcIdx < srcEnd) {
        if (TextCodec::isText(src[srcIdx])) {
            srcIdx++;
            continue;
        }

        if ((srcIdx > delimAnchor + 2) && TextCodec::isDelimiter(src[srcIdx])) { // At least 2 letters
            const byte val = src[delimAnchor + 1];
            const int length = srcIdx - delimAnchor - 1;

            if (length <= TextCodec::MAX_WORD_LENGTH) {
                // Compute hashes
                // h1 -> hash of word chars
                // h2 -> hash of word chars with first char case flipped
                int32 h1 = TextCodec::HASH1;
                h1 = h1 * TextCodec::HASH1 ^ int32(val) * TextCodec::HASH2;
                int32 h2 = TextCodec::HASH1;
                h2 = h2 * TextCodec::HASH1 ^ (int32(val) ^ 0x20) * TextCodec::HASH2;

                for (int i = delimAnchor + 2; i < srcIdx; i++) {
                    h1 = h1 * TextCodec::HASH1 ^ int32(src[i]) * TextCodec::HASH2;
                    h2 = h2 * TextCodec::HASH1 ^ int32(src[i]) * TextCodec::HASH2;
                }

                // Check word in dictionary
                DictEntry* pe = nullptr;
                prefetchRead(&_dictMap[h1 & _hashMask]);
                DictEntry* pe1 = _dictMap[h1 & _hashMask];

                // Check for hash collisions
                if ((pe1 != nullptr) && (pe1->_hash == h1) && ((pe1->_data >> 24) == length))
                    pe = pe1;

                if (pe == nullptr) {
                    prefetchRead(&_dictMap[h2 & _hashMask]);
                    DictEntry* pe2 = _dictMap[h2 & _hashMask];

                    if ((pe2 != nullptr) && (pe2->_hash == h2) && ((pe2->_data >> 24) == length))
                        pe = pe2;
                }

                if (pe != nullptr) {
                    if (!TextCodec::sameWords(pe->_ptr + 1, &src[delimAnchor + 2], length - 1))
                        pe = nullptr;
                }

                if (pe == nullptr) {
                    // Word not found in the dictionary or hash collision.
                    // Replace entry if not in static dictionary
                    if (((length > 3) || ((length > 2) && (words < TextCodec::THRESHOLD2))) && (pe1 == nullptr)) {
                        DictEntry* pe = &_dictList[words];

                        if ((pe->_data & TextCodec::MASK_LENGTH) >= _staticDictSize) {
                            // Reuse old entry
                            _dictMap[pe->_hash & _hashMask] = nullptr;
                            pe->_ptr = &src[delimAnchor + 1];
                            pe->_hash = h1;
                            pe->_data = (length << 24) | words;
                        }

                        // Update hash map
                        _dictMap[h1 & _hashMask] = pe;
                        words++;

                        // Dictionary full ? Expand or reset index to end of static dictionary
                        if (words >= _dictSize) {
                            if (expandDictionary() == false)
                                words = _staticDictSize;
                        }
                    }
                }
                else {
                    // Word found in the dictionary
                    // Skip space if only delimiter between 2 word references
                    if ((emitAnchor != delimAnchor) || (src[delimAnchor] != byte(' '))) {
                        const int dIdx = emitSymbols(&src[emitAnchor], &dst[dstIdx], delimAnchor + 1 - emitAnchor, dstEnd - dstIdx);

                        if (dIdx < 0) {
                            res = false;
                            break;
                        }

                        dstIdx += dIdx;
                    }

                    if (dstIdx >= dstEnd4) {
                        res = false;
                        break;
                    }

                    dst[dstIdx++] = (pe == pe1) ? TextCodec::ESCAPE_TOKEN1 : TextCodec::ESCAPE_TOKEN2;
                    dstIdx += emitWordIndex(&dst[dstIdx], pe->_data & TextCodec::MASK_LENGTH);
                    emitAnchor = delimAnchor + 1 + int(pe->_data >> 24);
                }
            }
        }

        // Reset delimiter position
        delimAnchor = srcIdx;
        srcIdx++;
    }

    if (res == true) {
        // Emit last symbols
        const int dIdx = emitSymbols(&src[emitAnchor], &dst[dstIdx], srcEnd - emitAnchor, dstEnd - dstIdx);

        if (dIdx < 0)
            res = false;
        else
            dstIdx += dIdx;

        res &= (srcIdx == srcEnd);
    }

    output._index += dstIdx;
    input._index += srcIdx;
    return res;
}

bool TextCodec1::expandDictionary()
{
    if (_dictSize >= TextCodec::MAX_DICT_SIZE)
        return false;

    DictEntry* newDict = new DictEntry[_dictSize * 2];
    memcpy(static_cast<void*>(&newDict[0]), &_dictList[0], sizeof(DictEntry) * _dictSize);

    for (int i = _dictSize; i < _dictSize * 2; i++)
        newDict[i] = DictEntry(nullptr, 0, i, 0);

    delete[] _dictList;
    _dictList = newDict;

    // Reset map (values must point to addresses of new DictEntry items)
    for (int i = 0; i < _dictSize; i++) {
        _dictMap[_dictList[i]._hash & _hashMask] = &_dictList[i];
    }

    _dictSize <<= 1;
    return true;
}

int TextCodec1::emitSymbols(byte src[], byte dst[], const int srcEnd, const int dstEnd)
{
    int dstIdx = 0;

    for (int i = 0; i < srcEnd; i++) {
        if (dstIdx >= dstEnd)
            return -1;

        const byte cur = src[i];

        switch (cur) {
        case TextCodec::ESCAPE_TOKEN1:
        case TextCodec::ESCAPE_TOKEN2: {
            // Emit special word
            dst[dstIdx++] = TextCodec::ESCAPE_TOKEN1;
            const int idx = (cur == TextCodec::ESCAPE_TOKEN1) ? _staticDictSize - 1 : _staticDictSize - 2;
            int lenIdx = 1;

            if (idx >= TextCodec::THRESHOLD1)
                lenIdx = (idx >= TextCodec::THRESHOLD2) ? 3 : 2;

            if (dstIdx + lenIdx >= dstEnd)
                return -1;

            dstIdx += emitWordIndex(&dst[dstIdx], idx);
            break;
        }

        case TextCodec::CR:
            if (_isCRLF == false)
                dst[dstIdx++] = cur;

            break;

        default:
            dst[dstIdx++] = cur;
        }
    }

    return dstIdx;
}

int TextCodec1::emitWordIndex(byte dst[], int val)
{
    // Emit word index (varint 5 bits + 7 bits + 7 bits)
    if (val >= TextCodec::THRESHOLD1) {
        int dstIdx = 0;

        if (val >= TextCodec::THRESHOLD2)
            dst[dstIdx++] = byte(0xE0 | (val >> 14));

        dst[dstIdx] = byte(0x80 | (val >> 7));
        dst[dstIdx + 1] = byte(0x7F & val);
        return dstIdx + 2;
    }

    dst[0] = byte(val);
    return 1;
}

bool TextCodec1::inverse(SliceArray<byte>& input, SliceArray<byte>& output, int count)
{
    byte* src = &input._array[input._index];
    byte* dst = &output._array[output._index];
    int srcIdx = 0;
    int dstIdx = 0;
    reset();
    const int srcEnd = count;
    const int dstEnd = output._length;
    int delimAnchor = TextCodec::isText(src[srcIdx]) ? srcIdx - 1 : srcIdx; // previous delimiter
    int words = _staticDictSize;
    bool wordRun = false;
    _isCRLF = int(src[srcIdx++] & TextCodec::MASK_CRLF) != 0;

    while ((srcIdx < srcEnd) && (dstIdx < dstEnd)) {
        byte cur = src[srcIdx];

        if (TextCodec::isText(cur)) {
            dst[dstIdx] = cur;
            srcIdx++;
            dstIdx++;
            continue;
        }

        if ((srcIdx > delimAnchor + 2) && TextCodec::isDelimiter(cur)) {
            const int length = srcIdx - delimAnchor - 1;

            if (length <= TextCodec::MAX_WORD_LENGTH) {
                int32 h1 = TextCodec::HASH1;

                for (int i = delimAnchor + 1; i < srcIdx; i++)
                    h1 = h1 * TextCodec::HASH1 ^ int32(src[i]) * TextCodec::HASH2;

                // Lookup word in dictionary
                DictEntry* pe = nullptr;
                DictEntry* pe1 = _dictMap[h1 & _hashMask];

                // Check for hash collisions
                if ((pe1 != nullptr) && (pe1->_hash == h1) && ((pe1->_data >> 24) == length)) {
                    if (TextCodec::sameWords(pe1->_ptr + 1, &src[delimAnchor + 2], length - 1))
                        pe = pe1;
                }

                if (pe == nullptr) {
                    // Word not found in the dictionary or hash collision.
                    // Replace entry if not in static dictionary
                    if (((length > 3) || ((length > 2) && (words < TextCodec::THRESHOLD2))) && (pe1 == nullptr)) {
                        DictEntry& e = _dictList[words];

                        if ((e._data & TextCodec::MASK_LENGTH) >= _staticDictSize) {
                            // Reuse old entry
                            _dictMap[e._hash & _hashMask] = nullptr;
                            e._ptr = &src[delimAnchor + 1];
                            e._hash = h1;
                            e._data = (length << 24) | words;
                        }

                        _dictMap[h1 & _hashMask] = &e;
                        words++;

                        // Dictionary full ? Expand or reset index to end of static dictionary
                        if (words >= _dictSize) {
                            if (expandDictionary() == false)
                                words = _staticDictSize;
                        }
                    }
                }
            }
        }

        srcIdx++;

        if ((cur == TextCodec::ESCAPE_TOKEN1) || (cur == TextCodec::ESCAPE_TOKEN2)) {
            // Word in dictionary
            // Read word index (varint 5 bits + 7 bits + 7 bits)
            int idx = int(src[srcIdx++]) & 0xFF;

            if ((idx & 0x80) != 0) {
                idx &= 0x7F;
                int idx2 = int(src[srcIdx++]);

                if ((idx2 & 0x80) != 0) {
                    idx = ((idx & 0x1F) << 7) | (idx2 & 0x7F);
                    idx2 = int(src[srcIdx++]);
                }

                idx = (idx << 7) | (idx2 & 0x7F);

                if (idx >= _dictSize)
                    break;
            }

            const int length = _dictList[idx]._data >> 24;
            const byte* buf = _dictList[idx]._ptr;

            // Sanity check
            if ((buf == nullptr) || (length > TextCodec::MAX_WORD_LENGTH) || (dstIdx + length >= dstEnd))
                break;

            // Emit word
            if (length > 1) {
                // Add space if only delimiter between 2 words (not an escaped delimiter)
                if (wordRun == true)
                    dst[dstIdx++] = TextCodec::SP;

                // Regular word entry
                wordRun = true;
                delimAnchor = srcIdx;
            }
            else {
                // Escape entry
                wordRun = false;
                delimAnchor = srcIdx - 1;
            }

            memcpy(&dst[dstIdx], &buf[0], length);

            // Flip case of first character ?
            if (cur == TextCodec::ESCAPE_TOKEN2)
                dst[dstIdx] ^= byte(0x20);

            dstIdx += length;
        }
        else {
            wordRun = false;
            delimAnchor = srcIdx - 1;

            if ((_isCRLF == true) && (cur == TextCodec::LF))
                dst[dstIdx++] = TextCodec::CR;

            dst[dstIdx++] = cur;
        }
    }

    output._index += dstIdx;
    input._index += srcIdx;
    return srcIdx == srcEnd;
}

TextCodec2::TextCodec2()
{
    _logHashSize = TextCodec::LOG_HASHES_SIZE;
    _dictSize = TextCodec::THRESHOLD2 * 4;
    _dictMap = nullptr;
    _dictList = nullptr;
    _hashMask = (1 << _logHashSize) - 1;
    _staticDictSize = TextCodec::STATIC_DICT_WORDS;
    _isCRLF = false;
}

TextCodec2::TextCodec2(Context& ctx)
{
    // Actual block size
    int blockSize = 0;
    int log = 13;
    int dSize = 1 << 12;

    if (ctx.has("blockSize")) {
        blockSize = ctx.getInt("blockSize");

        if (blockSize >= 8)
            log = max(min(Global::log2(blockSize / 8), 26), 13);

        // Select an appropriate initial dictionary size
        dSize = 1 << max(min(log - 4, 18), 12);
    }

    uint extraMem = 0;

    if (ctx.has("extra")) {
        string strExtra = ctx.getString("extra");
        extraMem = (strExtra == STR_TRUE) ? 1 : 0;
    }

    _logHashSize = log + extraMem;
    _dictSize = dSize;
    _dictMap = nullptr;
    _dictList = nullptr;
    _hashMask = (1 << _logHashSize) - 1;
    _staticDictSize = TextCodec::STATIC_DICT_WORDS;
    _isCRLF = false;
}

void TextCodec2::reset()
{
    const int mapSize = 1 << _logHashSize;

    if (_dictMap == nullptr)
        _dictMap = new DictEntry * [mapSize];

    for (int i = 0; i < mapSize; i++)
        _dictMap[i] = nullptr;

    if (_dictList == nullptr) {
        _dictList = new DictEntry[_dictSize];
        const int nbEntries = min(TextCodec::STATIC_DICT_WORDS, _dictSize);
        memcpy(static_cast<void*>(&_dictList[0]), &TextCodec::STATIC_DICTIONARY[0], nbEntries * sizeof(DictEntry));
    }

    for (int i = 0; i < _staticDictSize; i++)
        _dictMap[_dictList[i]._hash & _hashMask] = &_dictList[i];

    // Pre-allocate all dictionary entries
    for (int i = _staticDictSize; i < _dictSize; i++)
        _dictList[i] = DictEntry(nullptr, 0, i, 0);
}

bool TextCodec2::forward(SliceArray<byte>& input, SliceArray<byte>& output, int count)
{
    if (output._length - output._index < getMaxEncodedLength(count))
        return false;

    byte* src = &input._array[input._index];
    byte* dst = &output._array[output._index];
    int srcIdx = 0;
    int dstIdx = 0;

    int32 freqs[256] = { 0 };
    byte mode = TextCodec::computeStats(&src[srcIdx], count, freqs);

    // Not text ?
    if ((mode & TextCodec::MASK_NOT_TEXT) != byte(0))
        return false;

    reset();
    const int srcEnd = count;
    const int dstEnd = getMaxEncodedLength(count);
    const int dstEnd3 = dstEnd - 3;
    int emitAnchor = 0; // never less than 0
    int words = _staticDictSize;

    // DOS encoded end of line (CR+LF) ?
    _isCRLF = (mode & TextCodec::MASK_CRLF) != byte(0);
    dst[dstIdx++] = mode;
    bool res = true;

    while ((srcIdx < srcEnd) && (src[srcIdx] == TextCodec::SP)) {
        dst[dstIdx++] = TextCodec::SP;
        srcIdx++;
        emitAnchor++;
    }

    int delimAnchor = TextCodec::isText(src[srcIdx]) ? srcIdx - 1 : srcIdx; // previous delimiter

    while (srcIdx < srcEnd) {
        if (TextCodec::isText(src[srcIdx])) {
            srcIdx++;
            continue;
        }

        if ((srcIdx > delimAnchor + 2) && TextCodec::isDelimiter(src[srcIdx])) {
            const byte val = src[delimAnchor + 1];
            const int length = srcIdx - delimAnchor - 1;

            if (length <= TextCodec::MAX_WORD_LENGTH) {
                // Compute hashes
                // h1 -> hash of word chars
                // h2 -> hash of word chars with first char case flipped
                int32 h1 = TextCodec::HASH1;
                h1 = h1 * TextCodec::HASH1 ^ int32(val) * TextCodec::HASH2;
                int32 h2 = TextCodec::HASH1;
                h2 = h2 * TextCodec::HASH1 ^ (int32(val) ^ 0x20) * TextCodec::HASH2;

                for (int i = delimAnchor + 2; i < srcIdx; i++) {
                    h1 = h1 * TextCodec::HASH1 ^ int32(src[i]) * TextCodec::HASH2;
                    h2 = h2 * TextCodec::HASH1 ^ int32(src[i]) * TextCodec::HASH2;
                }

                // Check word in dictionary
                DictEntry* pe = nullptr;
                prefetchRead(&_dictMap[h1 & _hashMask]);
                DictEntry* pe1 = _dictMap[h1 & _hashMask];

                // Check for hash collisions
                if ((pe1 != nullptr) && (pe1->_hash == h1) && ((pe1->_data >> 24) == length))
                    pe = pe1;

                if (pe == nullptr) {
                    prefetchRead(&_dictMap[h2 & _hashMask]);
                    DictEntry* pe2 = _dictMap[h2 & _hashMask];

                    if ((pe2 != nullptr) && (pe2->_hash == h2) && ((pe2->_data >> 24) == length))
                        pe = pe2;
                }

                if (pe != nullptr) {
                    if (!TextCodec::sameWords(pe->_ptr + 1, &src[delimAnchor + 2], length - 1))
                        pe = nullptr;
                }

                if (pe == nullptr) {
                    // Word not found in the dictionary or hash collision.
                    // Replace entry if not in static dictionary
                    if (((length > 3) || ((length > 2) && (words < TextCodec::THRESHOLD2))) && (pe1 == nullptr)) {
                        DictEntry* pe = &_dictList[words];

                        if ((pe->_data & TextCodec::MASK_LENGTH) >= _staticDictSize) {
                            // Reuse old entry
                            _dictMap[pe->_hash & _hashMask] = nullptr;
                            pe->_ptr = &src[delimAnchor + 1];
                            pe->_hash = h1;
                            pe->_data = (length << 24) | words;
                        }

                        // Update hash map
                        _dictMap[h1 & _hashMask] = pe;
                        words++;

                        // Dictionary full ? Expand or reset index to end of static dictionary
                        if (words >= _dictSize) {
                            if (expandDictionary() == false)
                                words = _staticDictSize;
                        }
                    }
                }
                else {
                    // Word found in the dictionary
                    // Skip space if only delimiter between 2 word references
                    if ((emitAnchor != delimAnchor) || (src[delimAnchor] != TextCodec::SP)) {
                        const int dIdx = emitSymbols(&src[emitAnchor], &dst[dstIdx], delimAnchor + 1 - emitAnchor, dstEnd - dstIdx);

                        if (dIdx < 0) {
                            res = false;
                            break;
                        }

                        dstIdx += dIdx;
                    }

                    if (dstIdx >= dstEnd3) {
                        res = false;
                        break;
                    }

                    dstIdx += emitWordIndex(&dst[dstIdx], pe->_data & TextCodec::MASK_LENGTH, (pe == pe1) ? 0 : 32);
                    emitAnchor = delimAnchor + 1 + int(pe->_data >> 24);
                }
            }
        }

        // Reset delimiter position
        delimAnchor = srcIdx;
        srcIdx++;
    }

    if (res == true) {
        // Emit last symbols
        const int dIdx = emitSymbols(&src[emitAnchor], &dst[dstIdx], srcEnd - emitAnchor, dstEnd - dstIdx);

        if (dIdx < 0)
            res = false;
        else
            dstIdx += dIdx;

        res &= (srcIdx == srcEnd);
    }

    output._index += dstIdx;
    input._index += srcIdx;
    return res;
}

bool TextCodec2::expandDictionary()
{
    if (_dictSize >= TextCodec::MAX_DICT_SIZE)
        return false;

    DictEntry* newDict = new DictEntry[_dictSize * 2];
    memcpy(static_cast<void*>(&newDict[0]), &_dictList[0], sizeof(DictEntry) * _dictSize);

    for (int i = _dictSize; i < _dictSize * 2; i++)
        newDict[i] = DictEntry(nullptr, 0, i, 0);

    delete[] _dictList;
    _dictList = newDict;

    // Reset map (values must point to addresses of new DictEntry items)
    for (int i = 0; i < _dictSize; i++) {
        _dictMap[_dictList[i]._hash & _hashMask] = &_dictList[i];
    }

    _dictSize <<= 1;
    return true;
}

int TextCodec2::emitSymbols(byte src[], byte dst[], const int srcEnd, const int dstEnd)
{
    int dstIdx = 0;

    if (2 * srcEnd < dstEnd) {
        for (int i = 0; i < srcEnd; i++) {
            const byte cur = src[i];

            switch (cur) {
            case TextCodec::ESCAPE_TOKEN1:
                dst[dstIdx++] = TextCodec::ESCAPE_TOKEN1;
                dst[dstIdx++] = TextCodec::ESCAPE_TOKEN1;
                break;

            case TextCodec::CR:
                if (_isCRLF == false)
                    dst[dstIdx++] = cur;

                break;

            default:
                if ((cur & TextCodec::MASK_80) != byte(0))
                    dst[dstIdx++] = TextCodec::ESCAPE_TOKEN1;

                dst[dstIdx++] = cur;
            }
        }
    }
    else {
        for (int i = 0; i < srcEnd; i++) {
            const byte cur = src[i];

            switch (cur) {
            case TextCodec::ESCAPE_TOKEN1:
                if (dstIdx >= dstEnd - 1)
                    return -1;

                dst[dstIdx++] = TextCodec::ESCAPE_TOKEN1;
                dst[dstIdx++] = TextCodec::ESCAPE_TOKEN1;
                break;

            case TextCodec::CR:
                if (_isCRLF == false) {
                    if (dstIdx >= dstEnd)
                        return -1;

                    dst[dstIdx++] = cur;
                }

                break;

            default:
                if ((cur & TextCodec::MASK_80) != byte(0)) {
                    if (dstIdx >= dstEnd)
                        return -1;

                    dst[dstIdx++] = TextCodec::ESCAPE_TOKEN1;
                }

                if (dstIdx >= dstEnd)
                    return -1;

                dst[dstIdx++] = cur;
            }
        }
    }

    return dstIdx;
}

int TextCodec2::emitWordIndex(byte dst[], int val, int mask)
{
    // Emit word index (varint 5 bits + 7 bits + 7 bits)
    // 1st byte: 0x80 => word idx, 0x40 => more bytes, 0x20 => toggle case 1st symbol
    // 2nd byte: 0x80 => 1 more byte
    if (val >= TextCodec::THRESHOLD3) {
        if (val >= TextCodec::THRESHOLD4) {
            // 5 + 7 + 7 => 2^19
            dst[0] = byte(0xC0 | mask | ((val >> 14) & 0x1F));
            dst[1] = byte(0x80 | ((val >> 7) & 0x7F));
            dst[2] = byte(val & 0x7F);
            return 3;
        }

        // 5 + 7 => 2^12 = 32*128
        dst[0] = byte(0xC0 | mask | ((val >> 7) & 0x1F));
        dst[1] = byte(val & 0x7F);
        return 2;
    }

    // 5 => 32
    dst[0] = byte(0x80 | mask | val);
    return 1;
}

bool TextCodec2::inverse(SliceArray<byte>& input, SliceArray<byte>& output, int count)
{
    byte* src = &input._array[input._index];
    byte* dst = &output._array[output._index];
    int srcIdx = 0;
    int dstIdx = 0;
    reset();
    const int srcEnd = count;
    const int dstEnd = output._length;
    int delimAnchor = TextCodec::isText(src[srcIdx]) ? srcIdx - 1 : srcIdx; // previous delimiter
    int words = _staticDictSize;
    bool wordRun = false;
    _isCRLF = (src[srcIdx++] & TextCodec::MASK_CRLF) != byte(0);

    while ((srcIdx < srcEnd) && (dstIdx < dstEnd)) {
        byte cur = src[srcIdx];

        if (TextCodec::isText(cur)) {
            dst[dstIdx] = cur;
            srcIdx++;
            dstIdx++;
            continue;
        }

        if ((srcIdx > delimAnchor + 2) && TextCodec::isDelimiter(cur)) {
            const int length = srcIdx - delimAnchor - 1;

            if (length <= TextCodec::MAX_WORD_LENGTH) {
                int32 h1 = TextCodec::HASH1;

                for (int i = delimAnchor + 1; i < srcIdx; i++)
                    h1 = h1 * TextCodec::HASH1 ^ int32(src[i]) * TextCodec::HASH2;

                // Lookup word in dictionary
                DictEntry* pe = nullptr;
                DictEntry* pe1 = _dictMap[h1 & _hashMask];

                // Check for hash collisions
                if ((pe1 != nullptr) && (pe1->_hash == h1) && ((pe1->_data >> 24) == length)) {
                    if (TextCodec::sameWords(pe1->_ptr + 1, &src[delimAnchor + 2], length - 1))
                        pe = pe1;
                }

                if (pe == nullptr) {
                    // Word not found in the dictionary or hash collision.
                    // Replace entry if not in static dictionary
                    if (((length > 3) || ((length > 2) && (words < TextCodec::THRESHOLD2))) && (pe1 == nullptr)) {
                        DictEntry& e = _dictList[words];

                        if ((e._data & TextCodec::MASK_LENGTH) >= _staticDictSize) {
                            // Reuse old entry
                            _dictMap[e._hash & _hashMask] = nullptr;
                            e._ptr = &src[delimAnchor + 1];
                            e._hash = h1;
                            e._data = (length << 24) | words;
                        }

                        _dictMap[h1 & _hashMask] = &e;
                        words++;

                        // Dictionary full ? Expand or reset index to end of static dictionary
                        if (words >= _dictSize) {
                            if (expandDictionary() == false)
                                words = _staticDictSize;
                        }
                    }
                }
            }
        }

        srcIdx++;

        if ((cur & TextCodec::MASK_80) != byte(0)) {
            // Word in dictionary
            // Read word index (varint 5 bits + 7 bits + 7 bits)
            int idx = int(cur & TextCodec::MASK_1F);

            if ((cur & TextCodec::MASK_40) != byte(0)) {
                int idx2 = int(src[srcIdx++]);

                if ((idx2 & 0x80) != 0) {
                    idx = (idx << 7) | (idx2 & 0x7F);
                    idx2 = int(src[srcIdx++]);
                }

                idx = (idx << 7) | (idx2 & 0x7F);

                if (idx >= _dictSize)
                    break;
            }

            const int length = _dictList[idx]._data >> 24;
            const byte* buf = _dictList[idx]._ptr;

            // Sanity check
            if ((buf == nullptr) || (length > TextCodec::MAX_WORD_LENGTH) || (dstIdx + length >= dstEnd))
                break;

            // Emit word
            if (length > 1) {
                // Add space if only delimiter between 2 words (not an escaped delimiter)
                if (wordRun == true)
                    dst[dstIdx++] = TextCodec::SP;

                // Regular word entry
                wordRun = true;
                delimAnchor = srcIdx;
            }
            else {
                // Escape entry
                wordRun = false;
                delimAnchor = srcIdx - 1;
            }

            memcpy(&dst[dstIdx], &buf[0], length);

            // Flip case of first character ?
            if ((cur & TextCodec::MASK_20) != byte(0))
                dst[dstIdx] ^= byte(0x20);

            dstIdx += length;
        }
        else {
            if (cur == TextCodec::ESCAPE_TOKEN1) {
                dst[dstIdx++] = src[srcIdx++];
            }
            else {
                if ((_isCRLF == true) && (cur == TextCodec::LF))
                    dst[dstIdx++] = TextCodec::CR;

                dst[dstIdx++] = cur;
            }

            wordRun = false;
            delimAnchor = srcIdx - 1;
        }
    }

    output._index += dstIdx;
    input._index += srcIdx;
    return srcIdx == srcEnd;
}
