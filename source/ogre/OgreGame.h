#ifndef _OgreGame_h_
#define _OgreGame_h_

#include "BaseApp.h"
#include "common/Gui_Popup.h"
#include "common/SceneXml.h"
#include "common/BltObjects.h"
#include "common/TracksXml.h"
#include "common/FluidsXml.h"
#include "common/WaterRTT.h"
#include "ChampsXml.h"

#include "ReplayGame.h"
#include "../vdrift/cardefs.h"
#include "CarModel.h"
#include "CarReflection.h"

#include "common/MessageBox/MessageBox.h"
#include "common/MessageBox/MessageBoxStyle.h"
#include "common/GraphView.h"

#include "../network/networkcallbacks.hpp"
#include <boost/thread.hpp>
#include <MyGUI.h>
#include <OgreShadowCameraSetup.h>

#include "../shiny/Main/Factory.hpp"


namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class RenderWindow;  class Viewport;  class Light;
	class Terrain;  class TerrainGlobalOptions;  class TerrainGroup;  class TerrainPaging;  class PageManager;  }
namespace Forests {  class PagedGeometry;  }
namespace BtOgre  {  class DebugDrawer;  }
namespace MyGUI  {  class MultiList2;  }
namespace OISB   {  class AnalogAxisAction;  }
namespace MyGUI  {  class Slider;  }
class GraphView;
const int CarPosCnt = 8;  // size of poses queue


class App : public BaseApp, public GameClientCallback, public MasterClientCallback, public sh::MaterialListener
{
public:
	App(SETTINGS* settings, GAME* game);
	virtual ~App();
	void NullHUD();
	
	class GAME* pGame;  ///*
	void updatePoses(float time), newPoses(float time), newPerfTest(float time);
	void UpdThr();
	
	// stuff to be executed after BaseApp init
	void postInit(), SetFactoryDefaults();
	
	void setTranslations();
	
	std::vector<GraphView*> graphs;  /// graphs
	void CreateGraphs(),DestroyGraphs(),UpdateGraphs(),GraphsNewVals();
	int iEdTire, iTireLoad, iCurLat,iCurLong,iCurAlign, iUpdTireGr;  ///* tire edit */
	//  car perf test
	bool bPerfTest;  EPerfTest iPerfTestStage;
	void PerfLogVel(class CAR* pCar, float time);
	
	// This list holds new positions info for every CarModel
	PosInfo carPoses[CarPosCnt][8];  // [carsNum8]
	/*std::vector<int>*/ int iCurPoses[8];  // current index for carPoses queue
	std::map<int,int> carsCamNum;  // picked camera number for cars
	
	// Utility
	Ogre::Quaternion qFixCar,qFixWh;

	//  replay - full, user saves
	//  ghost - saved when best lap
	//  ghplay - ghost ride replay, loaded if was on disk
	//  frm - used when playing replay for hud and sounds
	Replay replay, ghost, ghplay;  ReplayFrame frm[4];
	Rewind rewind;  // to take car back in time (after crash etc.)
	const Ogre::String& GetGhostFile(std::string* ghCar=NULL);  std::string GetRplListDir();

	Scene* sc;  /// scene.xml
	FluidsXml fluidsXml;  /// fluid params xml
	BltObjects objs;  // veget collision in bullet
	Ogre::Light* sun;  void UpdFog(bool bForce=false), UpdSun();

	// vdrift static
	Ogre::StaticGeometry* mStaticGeom;
	
	// Weather  rain, snow
	Ogre::ParticleSystem *pr,*pr2;
	
	//  trees
	Forests::PagedGeometry *trees, *grass;
	
	Ogre::SceneNode* arrowNode; // checkpoint arrow
	Ogre::SceneNode* arrowRotNode; // seperate node for rotation
	Ogre::Quaternion arrowAnimStart, arrowAnimEnd, arrowAnimCur; // smooth animation
		
	void UpdateHUD(int carId, float time), ShowHUDvp(bool vp),
		SizeHUD(bool full, Ogre::Viewport* vp=NULL, int carId=-1);
	void UpdHUDRot(int baseCarId, int carId, float vel, float rpm);
	void GetHUDVals(int id, float* vel, float* rpm, float* clutch, int* gear);
	
	Ogre::SceneManager* sceneMgr() { return mSceneMgr; };

protected:
	boost::thread mThread;
	WaterRTT mWaterRTT;

	virtual void createScene();
	virtual void destroyScene();

	virtual bool frameStart(Ogre::Real time);  void DoNetworking();
	virtual bool frameEnd(Ogre::Real time);
	virtual bool keyPressed( const OIS::KeyEvent &arg );
	float fLastFrameDT;
		
	BtOgre::DebugDrawer *dbgdraw;  /// blt dbg
	void bltDumpRecursive(class CProfileIterator* profileIterator, int spacing, std::stringstream& os);
	void bltDumpAll(std::stringstream& os);

	//  mtr reload
	enum eMaterials {
		Mtr_CarBody, Mtr_CarInterior, Mtr_CarGlass,
		Mtr_CarTireFront, Mtr_CarTireRear,
		Mtr_Road,  NumMaterials  };
	Ogre::String sMtr[NumMaterials];

	///  HUD, 2D  ------------
	float asp, scX,scY, minX,maxX, minY,maxY;  // minimap visible range
	//  gear, vel
	MyGUI::TextBox *txGear[4],*txVel[4],*txBFuel[4];
	//  times bar
	MyGUI::TextBox *txTimTxt[4],*txTimes[4];  MyGUI::ImageBox *bckTimes[4];
	//  gauges
	Ogre::SceneNode *ndRpm[4], *ndVel[4], *ndRpmBk[4], *ndVelBk[4],*ndVelBm[4];
	Ogre::ManualObject* moRpm[4], *moVel[4], *moRpmBk[4], *moVelBk[4],*moVelBm[4];
	//  miniap
	Ogre::ManualObject* moMap[4];
	Ogre::SceneNode *ndMap[4], *ndLine;
	Ogre::SceneNode* vNdPos[4][5];  // car pos tris on minimap +1ghost
	Ogre::ManualObject* vMoPos[4][5];

	Ogre::ManualObject* Create2D(const Ogre::String& mat, Ogre::SceneManager* sceneMgr,
		Ogre::Real size, bool dyn = false, bool clr = false);

	Ogre::OverlayElement *hudCountdown,*hudNetMsg, *ovL[5],*ovR[5],*ovS[5], *ovU[5],*ovX[5],
		*hudAbs,*hudTcs, *hudWarnChk,*hudWonPlace, *hudOpp[5][3],*hudOppB;
	Ogre::Overlay *ovCountdown,*ovNetMsg, *ovCam, *ovWarnWin, *ovOpp, *ovAbsTcs, *ovCarDbg,*ovCarDbgTxt,*ovCarDbgExt;

	Ogre::String GetTimeString(float time) const;
	void CreateHUD(bool destroy), ShowHUD(bool hideAll=false), UpdMiniTer(), UpdDbgTxtClr();
	Ogre::Vector3 projectPoint(const Ogre::Camera* cam, const Ogre::Vector3& pos);  // 2d xy, z - out info
	MyGUI::TextBox* CreateNickText(int carId, Ogre::String text);


	///  create  . . . . . . . . . . . . . . . . . . . . . . . . 
	Ogre::String resCar, resTrk, resDrv;
	void CreateCar();
	void CreateTerrain(bool bNewHmap=false, bool bTer=true), CreateBltTerrain(), GetTerAngles(int xb=0,int yb=0,int xe=0,int ye=0, bool full=true);
	void CreateTrees(), CreateRoad(), CreateObjects(),DestroyObjects(bool clear);
	void CreateFluids(), CreateBltFluids(), UpdateWaterRTT(Ogre::Camera* cam), DestroyFluids();
	void CreateSkyDome(Ogre::String sMater, Ogre::Vector3 scale);
	void NewGame();  void NewGameDoLoad();  bool IsVdrTrack();  bool newGameRpl;
	//  fluids to destroy
	std::vector<Ogre::String/*MeshPtr*/> vFlSMesh;
	std::vector<Ogre::Entity*> vFlEnt;
	std::vector<Ogre::SceneNode*> vFlNd;
	// vdrift:
	void CreateVdrTrack(std::string strack, class TRACK* pTrack),
		CreateRacingLine(), CreateMinimap(), CreateRoadBezier();
public:
	static Ogre::ManualObject* CreateModel(Ogre::SceneManager* sceneMgr, const Ogre::String& mat,
		class VERTEXARRAY* a, Ogre::Vector3 vPofs, bool flip, bool track=false, const Ogre::String& name="");
protected:
	
	// Loading
	void LoadCleanUp(), LoadGame(), LoadScene(), LoadCar(), LoadTerrain(), LoadRoad(), LoadObjects(), LoadTrees(), LoadMisc();
	enum ELoadState { LS_CLEANUP=0, LS_GAME, LS_SCENE, LS_CAR, LS_TER, LS_ROAD, LS_OBJS, LS_TREES, LS_MISC, LS_ALL };
	
	// id, display name, initialised in App()
	// e.g.: 0, Cleaning up or 3, Loading scene
	std::map<unsigned int, std::string> loadingStates;
	// 1 behind map ( map.end() ): loading finished
	std::map<unsigned int, std::string>::iterator curLoadState;

	float mTimer;


	///  terrain
public:
	Ogre::Terrain* terrain; 
protected:
	Ogre::TerrainGlobalOptions* mTerrainGlobals;
	Ogre::TerrainGroup* mTerrainGroup;  bool mPaging;
	Ogre::TerrainPaging* mTerrainPaging;  Ogre::PageManager* mPageManager;
	//Vector3 getNormalAtWorldPosition(Terrain* terrain, Real x, Real z, Real s);

public:
	int iBlendMaps, blendMapSize;	bool noBlendUpd;  //  mtr from ter  . . . 
	char* blendMtr;  // mtr [blendMapSize x blendMapSize]
protected:
	void initBlendMaps(Ogre::Terrain* terrin, int xb=0,int yb=0, int xe=0,int ye=0, bool full=true);
	void configureTerrainDefaults(Ogre::Light* l), UpdTerErr();
	float Noise(float x, float zoom, int octaves, float persistance);
	float Noise(float x, float y, float zoom, int octaves, float persistance);
	//     xa  xb
	//1    .___.
	//0__./     \.___
	//   xa-s    xb+s
	inline float linRange(const float& x, const float& xa, const float& xb, const float& s)  // min, max, smooth range
	{
		if (x <= xa-s || x >= xb+s)  return 0.f;
		if (x >= xa && x <= xb)  return 1.f;
		if (x < xa)  return (x-xa)/s+1;
		if (x > xb)  return (xb-x)/s+1;
		return 0.f;
	}

public:
	void changeShadows(), UpdPSSMMaterials(), setMtrSplits(Ogre::String sMtrName);
	Ogre::Vector4 splitPoints;

protected:
	Ogre::ShadowCameraSetupPtr mPSSMSetup;
	void recreateReflections();  // call after refl_mode changed

	//  road
public:	
	class SplineRoad* road;
protected:

	///-----------------------------------------------------------------------------------------------------------------
	///  Gui
	///-----------------------------------------------------------------------------------------------------------------
	//  size
	void SizeGUI(); void doSizeGUI(MyGUI::EnumeratorWidgetPtr);
	std::vector<MyGUI::TabControl*> vSubTabsGame,vSubTabsOpts;

	//  shortcuts
	typedef MyGUI::WidgetPtr WP;
	typedef std::list <std::string> strlist;
	//  slider event and its text field for value
	#define SLV(name)  void sl##name(SL);  MyGUI::StaticTextPtr val##name;
	#define SL  MyGUI::Slider* wp, float val
	#define CMB MyGUI::ComboBox* wp, size_t val // combobox event args


	///  Gui common   --------------------------
	//  graphics
	SLV(Anisotropy);  SLV(ViewDist);  SLV(TerDetail);  SLV(TerDist);  SLV(RoadDist);
	SLV(TexSize);  SLV(TerMtr);  SLV(TerTripl);  // detail
	SLV(Trees);  SLV(Grass);  SLV(TreesDist);  SLV(GrassDist);  // paged
	SLV(Shaders);  SLV(ShadowType);  SLV(ShadowCount);  SLV(ShadowSize);  SLV(ShadowDist);  //SLV(ShadowFilter); // shadow
	SLV(WaterSize);  SLV(AntiAliasing); // screen
	void comboTexFilter(CMB), btnShadows(WP), btnShaders(WP), btnTrGrReset(WP),
		chkWaterReflect(WP), chkWaterRefract(WP),
		chkUseImposters(WP), chkImpostorsOnly(WP);
	void setOrigPos(WP wp, const char* relToWnd);

	//  tooltip
	WP mToolTip;  MyGUI::EditPtr mToolTipTxt;
	void setToolTips(MyGUI::EnumeratorWidgetPtr widgets);
	void notifyToolTip(MyGUI::Widget* sender, const MyGUI::ToolTipInfo& info);
	void boundedMove(MyGUI::Widget *moving, const MyGUI::IntPoint & point);

	//  language
	void comboLanguage(CMB);
	std::map<std::string, MyGUI::UString> languages; // <short name, display name>
	bool bGuiReinit;
	MyGUI::ButtonPtr bnQuit;  void btnQuit(WP);

	//  init
	void InitGui();  bool bGI;
	void GuiCenterMouse(),GuiInitTooltip(),GuiInitLang(), GuiInitGraphics(),GuiInitTrack();
	Ogre::String GetSceneryColor(Ogre::String name);
	void AddTrkL(std::string name, int user, const class TrackInfo* ti);

	//  track
	void UpdGuiRdStats(const SplineRoad* rd, const Scene* sc, const Ogre::String& sTrack, float time, bool champ=false),
		ReadTrkStats(), ReadTrkStatsChamp(Ogre::String track,bool reverse);
	MyGUI::MultiList2* trkList;  MyGUI::EditPtr trkDesc[2];
	MyGUI::StaticImagePtr imgPrv[2],imgMini[2],imgTer[2], imgTrkIco1,imgTrkIco2;
	const static int StTrk = 12, InfTrk = 11;
	MyGUI::StaticTextPtr valTrkNet, stTrk[2][StTrk], infTrk[2][InfTrk];  // [2] 2nd set is for champs

	void listTrackChng(MyGUI::MultiList2* li, size_t pos), TrackListUpd(bool resetNotFound=false);
	TracksXml tracksXml;  CarsXml carsXml;
	void btnTrkView1(WP),btnTrkView2(WP),ChangeTrackView();
	void updTrkListDim(), updChampListDim();
	const static int TcolW[32],TcolC[6],ChColW[8],StColW[8];
	const static Ogre::String clrsDiff[9],clrsRating[5],clrsLong[10];

	void edTrkFind(MyGUI::EditPtr),edRplFind(MyGUI::EditPtr);
	Ogre::String sTrkFind,sRplFind;  MyGUI::EditPtr edFind;
	strlist liTracks,liTracksUser;  void FillTrackLists();
	std::list<TrkL> liTrk;

	void CarListUpd(bool resetNotFound);
	void AddCarL(std::string name, const class CarInfo* ci);
	std::list<CarL> liCar;  void FillCarList();

	//  screen
	MyGUI::ListPtr resList;
	void InitGuiScreenRes(), btnResChng(WP), ResizeOptWnd();
	void chkVidFullscr(WP), chkVidVSync(WP), chkVidSSAA(WP);

	void comboGraphicsAll(CMB), comboRenderSystem(CMB);
		
	///-----------------------------------------

	//  main menu
	void toggleGui(bool toggle=true), GuiShortcut(WND_Types wnd, int tab, int subtab=-1);
	void UpdCarClrSld(bool upd=true);  bool bUpdCarClr;
	void MainMenuBtn(MyGUI::WidgetPtr), MenuTabChg(MyGUI::TabPtr, size_t);  bool loadReadme;

	///  championships
	ChampsXml champs;  ProgressXml progress;
	void ChampsXmlLoad(), ProgressSave(bool upgGui=true);
	void ChampNewGame(), ChampLoadEnd(), ChampsListUpdate(),
		ChampFillStageInfo(bool finished), ChampionshipAdvance(float timeCur);

	MyGUI::MultiList2* liChamps, *liStages, *liNetEnd;
	void listChampChng(MyGUI::MultiList2* li, size_t pos), listStageChng(MyGUI::MultiList2* li, size_t pos);
	void btnChampStart(WP), btnChampStageBack(WP), btnChampStageStart(WP), btnChampEndClose(WP), btnNetEndClose(WP);
	void btnStageNext(WP), btnStagePrev(WP);  MyGUI::StaticText* valStageNum;  MyGUI::ButtonPtr btChampStage;
	MyGUI::EditBox* edChampStage, *edChampEnd;  MyGUI::ImageBox* imgChampStage;
	void tabChampType(MyGUI::TabPtr wp, size_t id);
	

	///  input tab  -----------------------------------------
	void InitInputGui(), inputBindBtnClicked(WP);
	void InputBind(int key, int button=-1, int axis=-1);

	bool actionIsActive(std::string, std::string);
	void cmbJoystick(CMB), UpdateInputBars(), inputDetailBtn(WP);

	Ogre::String GetInputName(const Ogre::String& sName);
	//  joy events
	virtual bool axisMoved( const OIS::JoyStickEvent &e, int axis );
    virtual bool buttonPressed( const OIS::JoyStickEvent &e, int button );
    virtual bool buttonReleased( const OIS::JoyStickEvent &e, int button );
	MyGUI::StaticTextPtr txtJAxis, txtJBtn, txtInpDetail;  MyGUI::WidgetPtr panInputDetail;
	int lastAxis, axisCnt;  std::string joyName;  class OISB::AnalogAxisAction* actDetail;
	MyGUI::EditPtr edInputMin, edInputMax, edInputMul, edInputReturn, edInputIncrease;  void editInput(MyGUI::EditPtr);
	MyGUI::ComboBoxPtr cmbInpDetSet;  void comboInputPreset(CMB), comboInputKeyAllPreset(CMB);

	///  tweak
	const static int ciEdCar = 12;
	MyGUI::EditPtr edCar[ciEdCar],edPerfTest, edTweakCol;  MyGUI::TabPtr tabTweak, tabEdCar;
	MyGUI::StaticTextPtr txtTweakPath, txtTweakTire, txtTweakPathCol;
	MyGUI::ComboBoxPtr cmbTweakCarSet, cmbTweakTireSet;
	void CmbTweakCarSet(CMB), CmbTweakTireSet(CMB), CmbEdTweakCarSet(MyGUI::EditPtr), CmbEdTweakTireSet(MyGUI::EditPtr);
	void TweakToggle(), TweakCarSave(),TweakCarLoad(), TweakTireSave();
	void TweakColUpd(bool user), TweakColLoad(),TweakColSave();
	void btnTweakCarSave(WP),btnTweakCarLoad(WP), btnTweakTireSave(WP), btnTweakColSave(WP);
	void tabCarEdChng(MyGUI::TabPtr, size_t);
	//  graphs
	MyGUI::ComboBox* cmbGraphs;  void comboGraphs(CMB);  MyGUI::StaticTextPtr valGraphsType;


	//  sliders  -----------------------------------------
	SLV(Particles);  SLV(Trails);
	SLV(ReflSkip);  SLV(ReflSize);  SLV(ReflFaces);  SLV(ReflDist);  SLV(ReflMode); // refl
	SLV(SizeGaug);  SLV(TypeGaug);  SLV(SizeMinimap);  SLV(SizeArrow);  SLV(ZoomMinimap);
	SLV(CountdownTime);  // view
	SLV(DbgTxtClr);  SLV(DbgTxtCnt);
	SLV(VolMaster);  SLV(VolEngine);  SLV(VolTires);  SLV(VolSusp);  SLV(VolEnv);  // sounds
	SLV(VolFlSplash);  SLV(VolFlCont);  SLV(VolCarCrash);  SLV(VolCarScrap);
	
	SLV(CarClrH);  SLV(CarClrS);  SLV(CarClrV);  SLV(CarClrGloss);  SLV(CarClrRefl);  // car clr
	SLV(BloomInt);  SLV(BloomOrig);  SLV(BlurIntens);  // video
	SLV(DepthOfFieldFocus);  SLV(DepthOfFieldFar);  // dof
	SLV(HDRParam1);  SLV(HDRParam2);  SLV(HDRParam3);  // hdr
	SLV(HDRBloomInt);  SLV(HDRBloomOrig);  SLV(HDRAdaptationScale);
	SLV(HDRVignettingRadius);  SLV(HDRVignettingDarkness);
	SLV(NumLaps);  SLV(RplNumViewports);  // setup
	SLV(SSSEffect);  SLV(SSSVelFactor);  SLV(SteerRangeSurf);  SLV(SteerRangeSim);
	
	//  checks
	void chkGauges(WP),	chkArrow(WP),chkBeam(WP), chkDigits(WP),
		chkMinimap(WP), chkMiniZoom(WP), chkMiniRot(WP), chkMiniTer(WP), chkMiniBorder(WP),  // view
		chkFps(WP), chkWireframe(WP), 
		chkCamInfo(WP), chkTimes(WP), chkOpponents(WP), chkOpponentsSort(WP), chkCamTilt(WP),
		chkCarDbgBars(WP), chkCarDbgTxt(WP), chkCarDbgSurf(WP), chkGraphs(WP),
		chkBltDebug(WP), chkBltProfilerTxt(WP), chkProfilerTxt(WP),
		chkReverse(WP), chkParticles(WP), chkTrails(WP),
		chkAbs(WP), chkTcs(WP), chkGear(WP), chkRear(WP), chkRearInv(WP),  // car
		chkMouseCapture(WP), chkOgreDialog(WP), chkAutoStart(WP), chkEscQuits(WP),
		chkStartInMain(WP), chkBltLines(WP), chkLoadPics(WP), chkMultiThread(WP),  // startup
		chkVidEffects(WP), chkVidBloom(WP), chkVidHDR(WP), chkVidBlur(WP), UpdBloomVals(), chkVidSSAO(WP), // effects
		chkVidSoftParticles(WP), chkVidGodRays(WP), chkVidDepthOfField(WP), chkVidBoostFOV(WP),
		chkVegetCollis(WP), chkCarCollis(WP), chkRoadWCollis(WP), chkDynObjects(WP);  //game

	// gui car tire set gravel/asphalt
	int iTireSet;
	void tabTireSet(MyGUI::TabPtr wp, size_t id);
	MyGUI::ButtonPtr bchAbs,bchTcs;  MyGUI::Slider* slSSSEff,*slSSSVel,*slSteerRngSurf,*slSteerRngSim;

	void imgBtnCarClr(WP), btnCarClrRandom(WP), toggleWireframe();
	MyGUI::ButtonPtr bRkmh, bRmph;  void radKmh(WP), radMph(WP);
	MyGUI::ButtonPtr bRsimEasy, bRsimNorm;  void radSimEasy(WP), radSimNorm(WP);  bool bReloadSim;
	MyGUI::ButtonPtr chFps,chWire, chBlt,chBltTxt, chProfTxt, chDbgT,chDbgB,chDbgS, chGraphs,
		chTimes,chMinimp,chOpponents;

	///  replay  -----------------------------
	MyGUI::StaticTextPtr valRplPerc, valRplCur, valRplLen,
		valRplName,valRplInfo,valRplName2,valRplInfo2;
	MyGUI::Slider* slRplPos;  void slRplPosEv(SL);
	MyGUI::EditPtr edRplName, edRplDesc;
	void btnRplLoad(WP), btnRplSave(WP), btnRplDelete(WP), btnRplRename(WP),  // btn
		chkRplAutoRec(WP),chkRplChkGhost(WP),chkRplChkBestOnly(WP),
		chkRplChkAlpha(WP),chkRplChkPar(WP),chkRplChkRewind(WP),  // replay
		btnRplToStart(WP),btnRplToEnd(WP), btnRplPlay(WP),  // controls
		btnRplCur(WP),btnRplAll(WP),btnRplGhosts(WP);  // radio
	MyGUI::ButtonPtr rbRplCur, rbRplAll, rbRplGhosts;

	void btnRplBackDn(WP,int,int,MyGUI::MouseButton),btnRplBackUp(WP,int,int,MyGUI::MouseButton);
	void btnRplFwdDn(WP,int,int,MyGUI::MouseButton),btnRplFwdUp(WP,int,int,MyGUI::MouseButton);
	bool bRplBack,bRplFwd;
		
	void msgRplDelete(MyGUI::Message*, MyGUI::MessageBoxStyle);
	
	void btnNumPlayers(WP);  void chkSplitVert(WP);
	MyGUI::StaticTextPtr valLocPlayers;
	
	MyGUI::StaticTextPtr txCarStatsTxt,txCarStatsVals,
		txCarSpeed,txCarType, txCarAuthor,txTrackAuthor;
	void UpdCarStatsTxt();  // car stats


public:
	bool bRplPlay,bRplPause, bRplRec, bRplWnd;  //  game
	int carIdWin, iCurCar, iRplCarOfs;
protected:
	MyGUI::ButtonPtr btRplPl;  void UpdRplPlayBtn();
	///---------------------------------------

	//  game
	void btnNewGame(WP),btnNewGameStart(WP);
	MyGUI::MultiList2* carList;  MyGUI::ListPtr rplList;  void updReplaysList();
	void listRplChng(MyGUI::List* li, size_t pos),  changeCar();
	void listCarChng(MyGUI::MultiList2* li, size_t pos),  changeTrack();
	int LNext(MyGUI::MultiList2* lp, int rel, int ofs), LNext(MyGUI::ListPtr lp, int rel, int ofs),
		LNext(MyGUI::MultiList* lp, int rel);  // util next in list
	void LNext(int rel);  void tabPlayer(MyGUI::TabPtr wp, size_t id);

	Ogre::String sListCar,sListTrack;  int bListTrackU;
	Ogre::String pathTrk[2];  Ogre::String TrkDir();
	Ogre::String PathListTrk(int user=-1), PathListTrkPrv(int user/*=-1*/, Ogre::String track);

	MyGUI::StaticImagePtr imgCar;  MyGUI::EditPtr carDesc;
	void comboBoost(CMB), comboFlip(CMB);

	GuiPopup popup;

	///  multiplayer  ---------------------------------------
	void rebuildGameList(), rebuildPlayerList();
	void updateGameInfo(), updateGameSet(), updateGameInfoGUI();
	void setNetGuiHosting(bool enabled);
	void gameListChanged(protocol::GameList list);

	void peerConnected(PeerInfo peer), peerDisconnected(PeerInfo peer);
	void peerInfo(PeerInfo peer);
	void peerMessage(PeerInfo peer, std::string msg);
	void peerState(PeerInfo peer, uint8_t state);
	void gameInfo(protocol::GameInfo game);
	void startRace();
	void timeInfo(ClientID id, uint8_t lap, double time);
	void error(std::string what);
	void join(std::string host, std::string port, std::string password);
	void uploadGameInfo();

	mutable boost::mutex netGuiMutex;
	MyGUI::UString sChatBuffer,sChatLast1,sChatLast2;  int iChatMove;
	void AddChatMsg(const MyGUI::UString& clr, const MyGUI::UString& msg, bool add=true);
	protocol::GameInfo netGameInfo;

	bool bRebuildPlayerList, bRebuildGameList;
	bool bUpdateGameInfo, bStartGame, bUpdChat;

	///  multiplayer gui  --------------------
	MyGUI::TabPtr tabsNet;  //void tabNet(TabPtr tab, size_t id);
	MyGUI::WidgetPtr panelNetServer,panelNetGame,panelNetTrack;
	MyGUI::MultiListPtr listServers, listPlayers;
	MyGUI::EditPtr edNetChat;  // chat area, set text through sChatBuffer
	int iColLock,iColHost,iColPort;  //ids of columns in listServers, set in gui init

	MyGUI::ButtonPtr btnNetRefresh,btnNetJoin,btnNetCreate,btnNetDirect;
	MyGUI::ButtonPtr btnNetReady,btnNetLeave;
	void evBtnNetRefresh(WP);
	void evBtnNetJoin(WP), evBtnNetJoinLockedClose();
	void evBtnNetCreate(WP);
	void evBtnNetDirect(WP),evBtnNetDirectClose();
	void evBtnNetReady(WP),evBtnNetLeave(WP);

	MyGUI::StaticImagePtr imgNetTrack;
	MyGUI::StaticTextPtr valNetGames, valNetGameName, valNetChat, valNetTrack, valNetPassword;
	MyGUI::ButtonPtr btnNetSendMsg;  void chatSendMsg();
	MyGUI::EditPtr edNetGameName, edNetChatMsg, edNetTrackInfo, edNetPassword,
		edNetNick, edNetServerIP, edNetServerPort, edNetLocalPort;
	void evEdNetGameName(MyGUI::EditPtr), evEdNetPassword(MyGUI::EditPtr),
		evEdNetNick(MyGUI::EditPtr), evEdNetServerIP(MyGUI::EditPtr),
		evEdNetServerPort(MyGUI::EditPtr), evEdNetLocalPort(MyGUI::EditPtr);


public:
	virtual void materialCreated(sh::MaterialInstance* m, const std::string& configuration, unsigned short lodIndex);

	bool GetCarPath(std::string* pathCar/*out*/,
		std::string* pathSave/*=0*/, std::string* pathSaveDir/*=0*/,
		std::string carname, /*std::string tweakSetup="",*/ bool forceOrig=false);
};

#endif
