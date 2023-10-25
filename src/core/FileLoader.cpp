#include "common.h"
#include <ctype.h>
#include "main.h"

#include "General.h"
#include "Quaternion.h"
#include "ModelInfo.h"
#include "ModelIndices.h"
#include "TempColModels.h"
#include "VisibilityPlugins.h"
#include "FileMgr.h"
#include "HandlingMgr.h"
#include "CarCtrl.h"
#include "PedType.h"
#include "AnimManager.h"
#include "Game.h"
#include "RwHelper.h"
#include "NodeName.h"
#include "TxdStore.h"
#include "PathFind.h"
#include "ObjectData.h"
#include "DummyObject.h"
#include "World.h"
#include "Zones.h"
#include "ZoneCull.h"
#include "CdStream.h"
#include "FileLoader.h"
#include "MemoryHeap.h"
#include "Streaming.h"
#include "ColStore.h"
#include "Occlusion.h"
#ifdef NEW_VEHICLE_LOADER
#include "AudioSamples.h"
#endif

char CFileLoader::ms_line[256];

const char*
GetFilename(const char *filename)
{
	char *s = strrchr((char*)filename, '\\');
	return s ? s+1 : filename;
}

void
LoadingScreenLoadingFile(const char *filename)
{
	sprintf(gString, "Loading %s", GetFilename(filename));
	LoadingScreen("Loading the Game", gString, nil);
}

void
CFileLoader::LoadLevel(const char *filename)
{
	int fd;
	RwTexDictionary *savedTxd;
	bool objectsLoaded;
	char *line;
	char txdname[64];

	savedTxd = RwTexDictionaryGetCurrent();
	objectsLoaded = false;
	if(savedTxd == nil){
		savedTxd = RwTexDictionaryCreate();
		RwTexDictionarySetCurrent(savedTxd);
	}
	fd = CFileMgr::OpenFile(filename, "r");
	assert(fd > 0);

	for(line = LoadLine(fd); line; line = LoadLine(fd)){
		if(*line == '#')
			continue;

		if(strncmp(line, "EXIT", 4) == 0)
			break;

		if(strncmp(line, "IMAGEPATH", 9) == 0){
			RwImageSetPath(line + 10);
		}else if(strncmp(line, "TEXDICTION", 10) == 0){
			PUSH_MEMID(MEMID_TEXTURES);
			strcpy(txdname, line+11);
			LoadingScreenLoadingFile(txdname);
			RwTexDictionary *txd = LoadTexDictionary(txdname);
			AddTexDictionaries(savedTxd, txd);
			RwTexDictionaryDestroy(txd);
			POP_MEMID();
		}else if(strncmp(line, "COLFILE", 7) == 0){
			LoadingScreenLoadingFile(line+10);
			LoadCollisionFile(line+10, 0);
		}else if(strncmp(line, "MODELFILE", 9) == 0){
			LoadingScreenLoadingFile(line + 10);
			LoadModelFile(line + 10);
		}else if(strncmp(line, "HIERFILE", 8) == 0){
			LoadingScreenLoadingFile(line + 9);
			LoadClumpFile(line + 9);
		}else if(strncmp(line, "IDE", 3) == 0){
			LoadingScreenLoadingFile(line + 4);
			LoadObjectTypes(line + 4);
		}else if(strncmp(line, "IPL", 3) == 0){
			if(!objectsLoaded){
				LoadingScreenLoadingFile("Collision");
				PUSH_MEMID(MEMID_WORLD);
				CObjectData::Initialise("DATA\\OBJECT.DAT");
				CStreaming::Init();
				POP_MEMID();
				PUSH_MEMID(MEMID_COLLISION);
				CColStore::LoadAllCollision();
				POP_MEMID();
				for(int i = 0; i < MODELINFOSIZE; i++)
					if(CModelInfo::GetModelInfo(i))
						CModelInfo::GetModelInfo(i)->ConvertAnimFileIndex();
				objectsLoaded = true;
			}
			PUSH_MEMID(MEMID_WORLD);
			LoadingScreenLoadingFile(line + 4);
			LoadScene(line + 4);
			POP_MEMID();
		}else if(strncmp(line, "SPLASH", 6) == 0){
#ifndef DISABLE_LOADING_SCREEN
			LoadSplash(GetRandomSplashScreen());
#endif
#ifndef GTA_PS2
		}else if(strncmp(line, "CDIMAGE", 7) == 0){
			CdStreamAddImage(line + 8);
#endif
		}
	}

	CFileMgr::CloseFile(fd);
	RwTexDictionarySetCurrent(savedTxd);

	int i;
	for(i = 1; i < COLSTORESIZE; i++)
		if(CColStore::GetSlot(i))
			CColStore::GetBoundingBox(i).Grow(120.0f);
	CWorld::RepositionCertainDynamicObjects();
	CColStore::RemoveAllCollision();
}

char*
CFileLoader::LoadLine(int fd)
{
	int i;
	char *line;

	if(CFileMgr::ReadLine(fd, ms_line, 256) == false)
		return nil;
	for(i = 0; ms_line[i] != '\0'; i++)
		if(ms_line[i] < ' ' || ms_line[i] == ',')
			ms_line[i] = ' ';
	for(line = ms_line; *line <= ' ' && *line != '\0'; line++);
	return line;
}

RwTexDictionary*
CFileLoader::LoadTexDictionary(const char *filename)
{
	RwTexDictionary *txd;
	RwStream *stream;

	txd = nil;
	stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
	debug("Loading texture dictionary file %s\n", filename);
	if(stream){
		if(RwStreamFindChunk(stream, rwID_TEXDICTIONARY, nil, nil))
			txd = RwTexDictionaryGtaStreamRead(stream);
		RwStreamClose(stream, nil);
	}
	if(txd == nil)
		txd = RwTexDictionaryCreate();
	return txd;
}

struct ColHeader
{
	uint32 ident;
	uint32 size;
};

void
CFileLoader::LoadCollisionFile(const char *filename, uint8 colSlot)
{
	int fd;
	char modelname[24];
	CBaseModelInfo *mi;
	ColHeader header;

	PUSH_MEMID(MEMID_COLLISION);

	debug("Loading collision file %s\n", filename);
	fd = CFileMgr::OpenFile(filename, "rb");
	assert(fd > 0);

	while(CFileMgr::Read(fd, (char*)&header, sizeof(header))){
#ifdef VICE_CRY
		assert(header.ident == 'LLOC');
		CFileMgr::Read(fd, (char*)col_buff, header.size);
		memcpy(modelname, col_buff, 24);

		mi = CModelInfo::GetModelInfo(modelname, nil);
		if(mi){
			if(mi->GetColModel() && mi->DoesOwnColModel()){
				LoadCollisionModel(col_buff+24, *mi->GetColModel(), modelname);
			}else{
				CColModel *model = new CColModel;
				model->level = colSlot;
				LoadCollisionModel(col_buff+24, *model, modelname);
				mi->SetColModel(model, true);
			}
		}else{
			debug("colmodel %s can't find a modelinfo\n", modelname);
		}
#else
		assert(header.ident == 'LLOC');
		CFileMgr::Read(fd, (char*)work_buff, header.size);
		memcpy(modelname, work_buff, 24);

		mi = CModelInfo::GetModelInfo(modelname, nil);
		if(mi){
			if(mi->GetColModel() && mi->DoesOwnColModel()){
				LoadCollisionModel(work_buff+24, *mi->GetColModel(), modelname);
			}else{
				CColModel *model = new CColModel;
				model->level = colSlot;
				LoadCollisionModel(work_buff+24, *model, modelname);
				mi->SetColModel(model, true);
			}
		}else{
			debug("colmodel %s can't find a modelinfo\n", modelname);
		}
#endif
	}

	CFileMgr::CloseFile(fd);

	POP_MEMID();
}


bool
CFileLoader::LoadCollisionFileFirstTime(uint8 *buffer, uint32 size, uint8 colSlot)
{
	uint32 modelsize;
	char modelname[24];
	CBaseModelInfo *mi;
	ColHeader *header;
	int modelIndex;

	while(size > 8){
#ifdef VICE_CRY
		header = (ColHeader*)buffer;
		modelsize = header->size;
		if(header->ident != 'LLOC')
			return size-8 < CDSTREAM_SECTOR_SIZE;
		memcpy(modelname, buffer+8, 24);
		memcpy(col_buff, buffer+32, modelsize-24);
		size -= 32 + (modelsize-24);
		buffer += 32 + (modelsize-24);
		if(modelsize > 15*1024)
			debug("colmodel %s is huge, size %d\n", modelname, modelsize);

		mi = CModelInfo::GetModelInfo(modelname, &modelIndex);
		if(mi){
			CColStore::IncludeModelIndex(colSlot, modelIndex);
			CColModel *model = new CColModel;
			model->level = colSlot;
			LoadCollisionModel(col_buff, *model, modelname);
			mi->SetColModel(model, true);
		}else{
			debug("colmodel %s can't find a modelinfo\n", modelname);
		}
#else
		header = (ColHeader*)buffer;
		modelsize = header->size;
		if(header->ident != 'LLOC')
			return size-8 < CDSTREAM_SECTOR_SIZE;
		memcpy(modelname, buffer+8, 24);
		memcpy(work_buff, buffer+32, modelsize-24);
		size -= 32 + (modelsize-24);
		buffer += 32 + (modelsize-24);
		if(modelsize > 15*1024)
			debug("colmodel %s is huge, size %d\n", modelname, modelsize);

		mi = CModelInfo::GetModelInfo(modelname, &modelIndex);
		if(mi){
			CColStore::IncludeModelIndex(colSlot, modelIndex);
			CColModel *model = new CColModel;
			model->level = colSlot;
			LoadCollisionModel(work_buff, *model, modelname);
			mi->SetColModel(model, true);
		}else{
			debug("colmodel %s can't find a modelinfo\n", modelname);
		}
#endif
	}
	return true;
}

bool
CFileLoader::LoadCollisionFile(uint8 *buffer, uint32 size, uint8 colSlot)
{
	uint32 modelsize;
	char modelname[24];
	CBaseModelInfo *mi;
	ColHeader *header;

	while(size > 8){
#ifdef VICE_CRY
		header = (ColHeader*)buffer;
		modelsize = header->size;
		if(header->ident != 'LLOC')
			return size-8 < CDSTREAM_SECTOR_SIZE;
		memcpy(modelname, buffer+8, 24);
		memcpy(col_buff, buffer+32, modelsize-24);
		size -= 32 + (modelsize-24);
		buffer += 32 + (modelsize-24);
		if(modelsize > 15*1024)
			debug("colmodel %s is huge, size %d\n", modelname, modelsize);

		mi = CModelInfo::GetModelInfo(modelname, CColStore::GetSlot(colSlot)->minIndex, CColStore::GetSlot(colSlot)->maxIndex);
		if(mi){
			if(mi->GetColModel()){
				LoadCollisionModel(col_buff, *mi->GetColModel(), modelname);
			}else{
				CColModel *model = new CColModel;
				model->level = colSlot;
				LoadCollisionModel(col_buff, *model, modelname);
				mi->SetColModel(model, true);
			}
		}else{
			debug("colmodel %s can't find a modelinfo\n", modelname);
		}
#else
		header = (ColHeader*)buffer;
		modelsize = header->size;
		if(header->ident != 'LLOC')
			return size-8 < CDSTREAM_SECTOR_SIZE;
		memcpy(modelname, buffer+8, 24);
		memcpy(work_buff, buffer+32, modelsize-24);
		size -= 32 + (modelsize-24);
		buffer += 32 + (modelsize-24);
		if(modelsize > 15*1024)
			debug("colmodel %s is huge, size %d\n", modelname, modelsize);

		mi = CModelInfo::GetModelInfo(modelname, CColStore::GetSlot(colSlot)->minIndex, CColStore::GetSlot(colSlot)->maxIndex);
		if(mi){
			if(mi->GetColModel()){
				LoadCollisionModel(work_buff, *mi->GetColModel(), modelname);
			}else{
				CColModel *model = new CColModel;
				model->level = colSlot;
				LoadCollisionModel(work_buff, *model, modelname);
				mi->SetColModel(model, true);
			}
		}else{
			debug("colmodel %s can't find a modelinfo\n", modelname);
		}
#endif
	}
	return true;
}

void
CFileLoader::LoadCollisionModel(uint8 *buf, CColModel &model, char *modelname)
{
	int i;

	model.boundingSphere.radius = *(float*)(buf);
	model.boundingSphere.center.x = *(float*)(buf+4);
	model.boundingSphere.center.y = *(float*)(buf+8);
	model.boundingSphere.center.z = *(float*)(buf+12);
	model.boundingBox.min.x = *(float*)(buf+16);
	model.boundingBox.min.y = *(float*)(buf+20);
	model.boundingBox.min.z = *(float*)(buf+24);
	model.boundingBox.max.x = *(float*)(buf+28);
	model.boundingBox.max.y = *(float*)(buf+32);
	model.boundingBox.max.z = *(float*)(buf+36);
	model.numSpheres = *(int16*)(buf+40);
	buf += 44;
	if(model.numSpheres > 0){
		model.spheres = (CColSphere*)RwMalloc(model.numSpheres*sizeof(CColSphere));
		REGISTER_MEMPTR(&model.spheres);
		for(i = 0; i < model.numSpheres; i++){
			model.spheres[i].Set(*(float*)buf, *(CVector*)(buf+4), buf[16], buf[17]);
			buf += 20;
		}
	}else
		model.spheres = nil;

	model.numLines = *(int16*)buf;
	buf += 4;
	if(model.numLines > 0){
		//model.lines = (CColLine*)RwMalloc(model.numLines*sizeof(CColLine));
		REGISTER_MEMPTR(&model.lines);
		for(i = 0; i < model.numLines; i++){
			//model.lines[i].Set(*(CVector*)buf, *(CVector*)(buf+12));
			buf += 24;
		}
	}else
		model.lines = nil;
	model.numLines = 0;
	model.lines = nil;

	model.numBoxes = *(int16*)buf;
	buf += 4;
	if(model.numBoxes > 0){
		model.boxes = (CColBox*)RwMalloc(model.numBoxes*sizeof(CColBox));
		REGISTER_MEMPTR(&model.boxes);
		for(i = 0; i < model.numBoxes; i++){
			model.boxes[i].Set(*(CVector*)buf, *(CVector*)(buf+12), buf[24], buf[25]);
			buf += 28;
		}
	}else
		model.boxes = nil;

	int32 numVertices = *(int16*)buf;
	buf += 4;
	if(numVertices > 0){
		model.vertices = (CompressedVector*)RwMalloc(numVertices*sizeof(CompressedVector));
		REGISTER_MEMPTR(&model.vertices);
		for(i = 0; i < numVertices; i++){
			model.vertices[i].Set(*(float*)buf, *(float*)(buf+4), *(float*)(buf+8));
#if 0
			if(Abs(*(float*)buf) >= 256.0f ||
			   Abs(*(float*)(buf+4)) >= 256.0f ||
			   Abs(*(float*)(buf+8)) >= 256.0f)
				printf("%s:Collision volume too big\n", modelname);
#endif
			buf += 12;
		}
	}else
		model.vertices = nil;

	model.numTriangles = *(int16*)buf;
	buf += 4;
	if(model.numTriangles > 0){
		model.triangles = (CColTriangle*)RwMalloc(model.numTriangles*sizeof(CColTriangle));
		REGISTER_MEMPTR(&model.triangles);
		for(i = 0; i < model.numTriangles; i++){
			model.triangles[i].Set(*(int32*)buf, *(int32*)(buf+4), *(int32*)(buf+8), buf[12]);
			buf += 16;
		}
	}else
		model.triangles = nil;
}

static void
GetNameAndLOD(char *nodename, char *name, int *n)
{
	char *underscore = nil;
	for(char *s = nodename; *s != '\0'; s++){
		if(s[0] == '_' && (s[1] == 'l' || s[1] == 'L') && isdigit(s[2]))
			underscore = s;
	}
	if(underscore){
		strncpy(name, nodename, underscore - nodename);
		name[underscore - nodename] = '\0';
		*n = atoi(underscore + 2);
	}else{
		strncpy(name, nodename, 24);
		*n = 0;
	}
}

RpAtomic*
CFileLoader::FindRelatedModelInfoCB(RpAtomic *atomic, void *data)
{
	CSimpleModelInfo *mi;
	char *nodename, name[24];
	int n;
	RpClump *clump = (RpClump*)data;

	nodename = GetFrameNodeName(RpAtomicGetFrame(atomic));
	GetNameAndLOD(nodename, name, &n);
	mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(name, nil);
	if(mi){
		assert(mi->IsSimple());
		CVisibilityPlugins::SetAtomicRenderCallback(atomic, nil);
		mi->SetAtomic(n, atomic);
		RpClumpRemoveAtomic(clump, atomic);
		RpAtomicSetFrame(atomic, RwFrameCreate());
		CVisibilityPlugins::SetAtomicModelInfo(atomic, mi);
	}else{
		debug("Can't find Atomic %s\n", name);
	}

	return atomic;
}

#ifdef LIBRW
void
InitClump(RpClump *clump)
{
	RpClumpForAllAtomics(clump, ConvertPlatformAtomic, nil);
}
#else
#define InitClump(clump)
#endif

void
CFileLoader::LoadModelFile(const char *filename)
{
	RwStream *stream;
	RpClump *clump;

	debug("Loading model file %s\n", filename);
	stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
	if(RwStreamFindChunk(stream, rwID_CLUMP, nil, nil)){
		clump = RpClumpStreamRead(stream);
		if(clump){
			InitClump(clump);
			RpClumpForAllAtomics(clump, FindRelatedModelInfoCB, clump);
			RpClumpDestroy(clump);
		}
	}
	RwStreamClose(stream, nil);
}

void
CFileLoader::LoadClumpFile(const char *filename)
{
	RwStream *stream;
	RpClump *clump;
	char *nodename, name[24];
	int n;
	CClumpModelInfo *mi;

	debug("Loading model file %s\n", filename);
	stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
	while(RwStreamFindChunk(stream, rwID_CLUMP, nil, nil)){
		clump = RpClumpStreamRead(stream);
		if(clump){
			nodename = GetFrameNodeName(RpClumpGetFrame(clump));
			GetNameAndLOD(nodename, name, &n);
			mi = (CClumpModelInfo*)CModelInfo::GetModelInfo(name, nil);
			if(mi){
				InitClump(clump);
				assert(mi->IsClump());
				mi->SetClump(clump);
			}else
				RpClumpDestroy(clump);
		}
	}
	RwStreamClose(stream, nil);
}

bool
CFileLoader::LoadClumpFile(RwStream *stream, uint32 id)
{
	RpClump *clump;
	CClumpModelInfo *mi;

	if(!RwStreamFindChunk(stream, rwID_CLUMP, nil, nil))
		return false;
	clump = RpClumpStreamRead(stream);
	if(clump == nil)
		return false;
	InitClump(clump);
	mi = (CClumpModelInfo*)CModelInfo::GetModelInfo(id);
	mi->SetClump(clump);
	return true;
}

bool
CFileLoader::StartLoadClumpFile(RwStream *stream, uint32 id)
{
	if(RwStreamFindChunk(stream, rwID_CLUMP, nil, nil)){
		printf("Start loading %s\n", CModelInfo::GetModelInfo(id)->GetModelName());
		return RpClumpGtaStreamRead1(stream);
	}else{
		printf("FAILED\n");
		return false;
	}
}

bool
CFileLoader::FinishLoadClumpFile(RwStream *stream, uint32 id)
{
	RpClump *clump;
	CClumpModelInfo *mi;

	printf("Finish loading %s\n", CModelInfo::GetModelInfo(id)->GetModelName());
	clump = RpClumpGtaStreamRead2(stream);

	if(clump){
		InitClump(clump);
		mi = (CClumpModelInfo*)CModelInfo::GetModelInfo(id);
		mi->SetClump(clump);
		return true;
	}else{
		printf("FAILED\n");
		return false;
	}
}

CSimpleModelInfo *gpRelatedModelInfo;

bool
CFileLoader::LoadAtomicFile(RwStream *stream, uint32 id)
{
	RpClump *clump;

	if(RwStreamFindChunk(stream, rwID_CLUMP, nil, nil)){
		clump = RpClumpStreamRead(stream);
		if(clump == nil)
			return false;
		InitClump(clump);
		gpRelatedModelInfo = (CSimpleModelInfo*)CModelInfo::GetModelInfo(id);
		RpClumpForAllAtomics(clump, SetRelatedModelInfoCB, clump);
		RpClumpDestroy(clump);
	}
	return true;
}

RpAtomic*
CFileLoader::SetRelatedModelInfoCB(RpAtomic *atomic, void *data)
{
	char *nodename, name[24];
	int n;
	RpClump *clump = (RpClump*)data;

	nodename = GetFrameNodeName(RpAtomicGetFrame(atomic));
	GetNameAndLOD(nodename, name, &n);
	CVisibilityPlugins::SetAtomicRenderCallback(atomic, nil);
	gpRelatedModelInfo->SetAtomic(n, atomic);
	RpClumpRemoveAtomic(clump, atomic);
	RpAtomicSetFrame(atomic, RwFrameCreate());
	CVisibilityPlugins::SetAtomicModelInfo(atomic, gpRelatedModelInfo);
	return atomic;
}

RpClump*
CFileLoader::LoadAtomicFile2Return(const char *filename)
{
	RwStream *stream;
	RpClump *clump;

	clump = nil;
	debug("Loading model file %s\n", filename);
	stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
	if(RwStreamFindChunk(stream, rwID_CLUMP, nil, nil))
		clump = RpClumpStreamRead(stream);
	if(clump)
		InitClump(clump);
	RwStreamClose(stream, nil);
	return clump;
}

static RwTexture*
MoveTexturesCB(RwTexture *texture, void *pData)
{
	RwTexDictionaryAddTexture((RwTexDictionary*)pData, texture);
	return texture;
}

void
CFileLoader::AddTexDictionaries(RwTexDictionary *dst, RwTexDictionary *src)
{
	RwTexDictionaryForAllTextures(src, MoveTexturesCB, dst);
}

#define isLine3(l, a, b, c) ((l[0] == a) && (l[1] == b) && (l[2] == c))
#define isLine4(l, a, b, c, d) ((l[0] == a) && (l[1] == b) && (l[2] == c) && (l[3] == d))

void
CFileLoader::LoadObjectTypes(const char *filename)
{
	enum {
		NONE,
		OBJS,
		MLO,	// unused but enum still has it
		TOBJ,
		WEAP,
		HIER,
		CARS,
		PEDS,
		PATH,
		TWODFX,
#ifdef NEW_VEHICLE_LOADER
		NEWCARS,
#endif
	};
	char *line;
	int fd;
	int section;
	int pathIndex;
	int id, pathType;
	int minID, maxID;

	section = NONE;
	minID = INT32_MAX;
	maxID = -1;
	pathIndex = -1;
	debug("Loading object types from %s...\n", filename);

	fd = CFileMgr::OpenFile(filename, "rb");
	assert(fd > 0);
	for(line = CFileLoader::LoadLine(fd); line; line = CFileLoader::LoadLine(fd)){
		if(*line == '\0' || *line == '#')
			continue;

		if(section == NONE){
			if(isLine4(line, 'o','b','j','s')) section = OBJS;
			else if(isLine4(line, 't','o','b','j')) section = TOBJ;
			else if(isLine4(line, 'w','e','a','p')) section = WEAP;
			else if(isLine4(line, 'h','i','e','r')) section = HIER;
			else if(isLine4(line, 'c','a','r','s')) section = CARS;
			else if(isLine4(line, 'p','e','d','s')) section = PEDS;
			else if(isLine4(line, 'p','a','t','h')) section = PATH;
			else if(isLine4(line, '2','d','f','x')) section = TWODFX;
#ifdef NEW_VEHICLE_LOADER
			else if(isLine4(line, 'n','e','w','c')) section = NEWCARS;
#endif
		}else if(isLine3(line, 'e','n','d')){
			section = NONE;
		}else switch(section){
		case OBJS:
			id = LoadObject(line);
			if(id > maxID) maxID = id;
			if(id < minID) minID = id;
			break;
		case TOBJ:
			id = LoadTimeObject(line);
			if(id > maxID) maxID = id;
			if(id < minID) minID = id;
			break;
		case WEAP:
			LoadWeaponObject(line);
			break;
		case HIER:
			LoadClumpObject(line);
			break;
		case CARS:
			LoadVehicleObject(line);
			break;
		case PEDS:
			LoadPedObject(line);
			break;
		case PATH:
			if(pathIndex == -1){
				id = LoadPathHeader(line, pathType);
				pathIndex = 0;
			}else{
				if(pathType == 0)
					LoadPedPathNode(line, id, pathIndex);
				else if (pathType == 1)
					LoadCarPathNode(line, id, pathIndex, false);
				else if (pathType == 2)
					LoadCarPathNode(line, id, pathIndex, true);
				pathIndex++;
				if(pathIndex == 12)
					pathIndex = -1;
			}
			break;
		case TWODFX:
			Load2dEffect(line);
			break;
#ifdef NEW_VEHICLE_LOADER
		case NEWCARS:
			LoadAnotherVehicleObject(line);
			break;
#endif
		}
	}
	CFileMgr::CloseFile(fd);

	for(id = minID; id <= maxID; id++){
		CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(id);
		if(mi && mi->IsBuilding())
			mi->SetupBigBuilding(minID, maxID);
	}
}

void
SetModelInfoFlags(CSimpleModelInfo *mi, uint32 flags)
{
	mi->m_wetRoadReflection =	!!(flags & 1);
	mi->m_noFade =	!!(flags & 2);
	mi->m_drawLast =	!!(flags & (4|8));
	mi->m_additive =	!!(flags & 8);
	mi->m_isSubway =	!!(flags & 0x10);
	mi->m_ignoreLight =	!!(flags & 0x20);
	mi->m_noZwrite =	!!(flags & 0x40);
	mi->m_noShadows =	!!(flags & 0x80);
	mi->m_ignoreDrawDist =	!!(flags & 0x100);
	mi->m_isCodeGlass =	!!(flags & 0x200);
	mi->m_isArtistGlass =	!!(flags & 0x400);
}

int
CFileLoader::LoadObject(const char *line)
{
	int id, numObjs;
	char model[24], txd[24];
	float dist[3];
	uint32 flags;
	int damaged;
	CSimpleModelInfo *mi;

	if(sscanf(line, "%d %s %s %d", &id, model, txd, &numObjs) != 4)
		return 0;	// game returns return value

	switch(numObjs){
	case 1:
		sscanf(line, "%d %s %s %d %f %d",
			&id, model, txd, &numObjs, &dist[0], &flags);
		damaged = 0;
		break;
	case 2:
		sscanf(line, "%d %s %s %d %f %f %d",
			&id, model, txd, &numObjs, &dist[0], &dist[1], &flags);
		damaged = dist[0] < dist[1] ?	// Are distances increasing?
			0 :	// Yes, no damage model
			1;	// No, 1 is damaged
		break;
	case 3:
		sscanf(line, "%d %s %s %d %f %f %f %d",
			&id, model, txd, &numObjs, &dist[0], &dist[1], &dist[2], &flags);
		damaged = dist[0] < dist[1] ?	// Are distances increasing?
				(dist[1] < dist[2] ? 0 : 2) :	// Yes, only 2 can still be a damage model
			1;	// No, 1 and 2 are damaged
		break;
	}

	mi = CModelInfo::AddSimpleModel(id);
	mi->SetModelName(model);
	mi->SetNumAtomics(numObjs);
	mi->SetLodDistances(dist);
	SetModelInfoFlags(mi, flags);
	mi->m_firstDamaged = damaged;
	mi->SetTexDictionary(txd);
	MatchModelString(model, id);

	return id;
}

int
CFileLoader::LoadTimeObject(const char *line)
{
	int id, numObjs;
	char model[24], txd[24];
	float dist[3];
	uint32 flags;
	int timeOn, timeOff;
	int damaged;
	CTimeModelInfo *mi, *other;

	if(sscanf(line, "%d %s %s %d", &id, model, txd, &numObjs) != 4)
		return 0;	// game returns return value

	switch(numObjs){
	case 1:
		sscanf(line, "%d %s %s %d %f %d %d %d",
			&id, model, txd, &numObjs, &dist[0], &flags, &timeOn, &timeOff);
		damaged = 0;
		break;
	case 2:
		sscanf(line, "%d %s %s %d %f %f %d %d %d",
			&id, model, txd, &numObjs, &dist[0], &dist[1], &flags, &timeOn, &timeOff);
		damaged = dist[0] < dist[1] ?	// Are distances increasing?
			0 :	// Yes, no damage model
			1;	// No, 1 is damaged
		break;
	case 3:
		sscanf(line, "%d %s %s %d %f %f %f %d %d %d",
			&id, model, txd, &numObjs, &dist[0], &dist[1], &dist[2], &flags, &timeOn, &timeOff);
		damaged = dist[0] < dist[1] ?	// Are distances increasing?
				(dist[1] < dist[2] ? 0 : 2) :	// Yes, only 2 can still be a damage model
			1;	// No, 1 and 2 are damaged
		break;
	}

	mi = CModelInfo::AddTimeModel(id);
	mi->SetModelName(model);
	mi->SetNumAtomics(numObjs);
	mi->SetLodDistances(dist);
	SetModelInfoFlags(mi, flags);
	mi->m_firstDamaged = damaged;
	mi->SetTimes(timeOn, timeOff);
	mi->SetTexDictionary(txd);
	other = mi->FindOtherTimeModel();
	if(other)
		other->SetOtherTimeModel(id);
	MatchModelString(model, id);

	return id;
}

int
CFileLoader::LoadWeaponObject(const char *line)
{
	int id, numObjs;
	char model[24], txd[24], animFile[16];
	float dist;
	CWeaponModelInfo *mi;

	sscanf(line, "%d %s %s %s %d %f", &id, model, txd, animFile, &numObjs, &dist);

	mi = CModelInfo::AddWeaponModel(id);
	mi->SetModelName(model);
	mi->SetNumAtomics(1);
	mi->m_lodDistances[0] = dist;
	mi->SetTexDictionary(txd);
	mi->SetAnimFile(animFile);
	mi->SetColModel(&CTempColModels::ms_colModelWeapon);
	MatchModelString(model, id);
	return id;
}

void
CFileLoader::LoadClumpObject(const char *line)
{
	int id;
	char model[24], txd[24];
	CClumpModelInfo *mi;

	if(sscanf(line, "%d %s %s", &id, model, txd) == 3){
		mi = CModelInfo::AddClumpModel(id);
		mi->SetModelName(model);
		mi->SetTexDictionary(txd);
		mi->SetColModel(&CTempColModels::ms_colModelBBox);
	}
}

void
CFileLoader::LoadVehicleObject(const char *line)
{
	int id;
	char model[24], txd[24];
	char type[8], handlingId[16], gamename[32], animFile[16], vehclass[12];
	uint32 frequency, comprules;
	int32 level, misc;
	float wheelScale;
	CVehicleModelInfo *mi;
	char *p;

	sscanf(line, "%d %s %s %s %s %s %s %s %d %d %x %d %f",
		&id, model, txd,
		type, handlingId, gamename, animFile, vehclass,
		&frequency, &level, &comprules, &misc, &wheelScale);

	mi = CModelInfo::AddVehicleModel(id);
	mi->SetModelName(model);
	mi->SetTexDictionary(txd);
	mi->SetAnimFile(animFile);
	for(p = gamename; *p; p++)
		if(*p == '_') *p = ' ';
	strcpy(mi->m_gameName, gamename);
	mi->m_level = level;
	mi->m_compRules = comprules;

	if(strcmp(type, "car") == 0){
		mi->m_wheelId = misc;
		mi->m_wheelScale = wheelScale;
		mi->m_vehicleType = VEHICLE_TYPE_CAR;
	}else if(strcmp(type, "boat") == 0){
		mi->m_vehicleType = VEHICLE_TYPE_BOAT;
	}else if(strcmp(type, "train") == 0){
		mi->m_vehicleType = VEHICLE_TYPE_TRAIN;
	}else if(strcmp(type, "heli") == 0){
		mi->m_vehicleType = VEHICLE_TYPE_HELI;
	}else if(strcmp(type, "plane") == 0){
		mi->m_planeLodId = misc;
		mi->m_wheelScale = 1.0f;
		mi->m_vehicleType = VEHICLE_TYPE_PLANE;
	}else if(strcmp(type, "bike") == 0){
		mi->m_bikeSteerAngle = misc;
		mi->m_wheelScale = wheelScale;
		mi->m_vehicleType = VEHICLE_TYPE_BIKE;
	}else
		assert(0);

	mi->m_handlingId = mod_HandlingManager.GetHandlingId(handlingId);

	if(strcmp(vehclass, "normal") == 0)
		mi->m_vehicleClass = CCarCtrl::NORMAL;
	else if(strcmp(vehclass, "poorfamily") == 0)
		mi->m_vehicleClass = CCarCtrl::POOR;
	else if(strcmp(vehclass, "richfamily") == 0)
		mi->m_vehicleClass = CCarCtrl::RICH;
	else if(strcmp(vehclass, "executive") == 0)
		mi->m_vehicleClass = CCarCtrl::EXEC;
	else if(strcmp(vehclass, "worker") == 0)
		mi->m_vehicleClass = CCarCtrl::WORKER;
	else if(strcmp(vehclass, "big") == 0)
		mi->m_vehicleClass = CCarCtrl::BIG;
	else if(strcmp(vehclass, "taxi") == 0)
		mi->m_vehicleClass = CCarCtrl::TAXI;
	else if(strcmp(vehclass, "moped") == 0)
		mi->m_vehicleClass = CCarCtrl::MOPED;
	else if(strcmp(vehclass, "motorbike") == 0)
		mi->m_vehicleClass = CCarCtrl::MOTORBIKE;
	else if(strcmp(vehclass, "leisureboat") == 0)
		mi->m_vehicleClass = CCarCtrl::LEISUREBOAT;
	else if(strcmp(vehclass, "workerboat") == 0)
		mi->m_vehicleClass = CCarCtrl::WORKERBOAT;
	else if(strcmp(vehclass, "ignore") == 0) {
		mi->m_vehicleClass = -1;
		return;
	}
	CCarCtrl::AddToCarArray(id, mi->m_vehicleClass);
	mi->m_frequency = frequency;
}

void
CFileLoader::LoadPedObject(const char *line)
{
	int id;
	char model[24], txd[24];
	char pedType[24], pedStats[24], animGroup[24], animFile[16];
	int carsCanDrive;
	CPedModelInfo *mi;
	int animGroupId;
	int radio1, radio2;

	sscanf(line, "%d %s %s %s %s %s %x %s %d %d",
	          &id, model, txd,
	          pedType, pedStats, animGroup, &carsCanDrive,
		  animFile, &radio1, &radio2);

	mi = CModelInfo::AddPedModel(id);
	mi->SetModelName(model);
	mi->SetTexDictionary(txd);
	mi->SetAnimFile(animFile);
	mi->SetColModel(&CTempColModels::ms_colModelPed1);
	mi->m_pedType = CPedType::FindPedType(pedType);
	mi->m_pedStatType = CPedStats::GetPedStatType(pedStats);
	for(animGroupId = 0; animGroupId < NUM_ANIM_ASSOC_GROUPS; animGroupId++)
		if(strcmp(animGroup, CAnimManager::GetAnimGroupName((AssocGroupId)animGroupId)) == 0)
			break;
	assert(animGroupId < NUM_ANIM_ASSOC_GROUPS);
	mi->m_animGroup = animGroupId;
	mi->m_carsCanDrive = carsCanDrive;
	mi->radio1 = radio1;
	mi->radio2 = radio2;
}

int
CFileLoader::LoadPathHeader(const char *line, int &type)
{
	int id;
	char modelname[32];

	sscanf(line, "%d %d %s", &type, &id, modelname);
	return id;
}

void
CFileLoader::LoadPedPathNode(const char *line, int id, int node)
{
	int type, next, cross, numLeft, numRight, speed, flags;
	float x, y, z, width, spawnRate;

	if(sscanf(line, "%d %d %d %f %f %f %f %d %d %d %d %f",
			&type, &next, &cross, &x, &y, &z, &width, &numLeft, &numRight,
			&speed, &flags, &spawnRate) != 12)
		spawnRate = 1.0f;

	if(id == -1)
		ThePaths.StoreDetachedNodeInfoPed(node, type, next, x, y, z,
			width, !!cross, !!(flags&1), !!(flags&4), spawnRate*15.0f);
	else
		ThePaths.StoreNodeInfoPed(id, node, type, next, x, y, z,
			width, !!cross, spawnRate*15.0f);
}

void
CFileLoader::LoadCarPathNode(const char *line, int id, int node, bool waterPath)
{
	int type, next, cross, numLeft, numRight, speed, flags;
	float x, y, z, width, spawnRate;

	if(sscanf(line, "%d %d %d %f %f %f %f %d %d %d %d %f",
			&type, &next, &cross, &x, &y, &z, &width, &numLeft, &numRight,
			&speed, &flags, &spawnRate) != 12)
		spawnRate = 1.0f;

	if(id == -1)
		ThePaths.StoreDetachedNodeInfoCar(node, type, next, x, y, z, width, numLeft, numRight,
			!!(flags&1), !!(flags&4), speed, !!(flags&2), waterPath, spawnRate * 15, false);
	else
		ThePaths.StoreNodeInfoCar(id, node, type, next, x, y, z, 0, numLeft, numRight,
			!!(flags&1), !!(flags&4), speed, !!(flags&2), waterPath, spawnRate * 15);
}


void
CFileLoader::Load2dEffect(const char *line)
{
	int id, r, g, b, a, type, ptype;
	float x, y, z;
	char corona[32], shadow[32];
	int shadowIntens, lightType, roadReflection, flare, flags, probability;
	CBaseModelInfo *mi;
	C2dEffect *effect;
	char *p;

	sscanf(line, "%d %f %f %f %d %d %d %d %d", &id, &x, &y, &z, &r, &g, &b, &a, &type);

	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(CTxdStore::FindTxdSlot("particle"));

	mi = CModelInfo::GetModelInfo(id);
	effect = CModelInfo::Get2dEffectStore().Alloc();
	mi->Add2dEffect(effect);
	effect->pos = CVector(x, y, z);
	effect->col = CRGBA(r, g, b, a);
	effect->type = type;

	switch(effect->type){
	case EFFECT_LIGHT:
		while(*line++ != '"');
		p = corona;
		while(*line != '"') *p++ = *line++;
		*p = '\0';
		line++;

		while(*line++ != '"');
		p = shadow;
		while(*line != '"') *p++ = *line++;
		*p = '\0';
		line++;

		sscanf(line, "%f %f %f %f %d %d %d %d %d",
			&effect->light.dist,
			&effect->light.range,
			&effect->light.size,
			&effect->light.shadowSize,
			&shadowIntens, &lightType, &roadReflection, &flare, &flags);
		effect->light.corona = RwTextureRead(corona, nil);
		effect->light.shadow = RwTextureRead(shadow, nil);
		effect->light.shadowIntensity = shadowIntens;
		effect->light.lightType = lightType;
		effect->light.roadReflection = roadReflection;
		effect->light.flareType = flare;

		if(flags & LIGHTFLAG_FOG_ALWAYS)
			flags &= ~LIGHTFLAG_FOG_NORMAL;
		effect->light.flags = flags;
		break;

	case EFFECT_PARTICLE:
		sscanf(line, "%d %f %f %f %d %d %d %d %d %d %f %f %f %f",
			&id, &x, &y, &z, &r, &g, &b, &a, &type,
			&effect->particle.particleType,
			&effect->particle.dir.x,
			&effect->particle.dir.y,
			&effect->particle.dir.z,
			&effect->particle.scale);
		break;

	case EFFECT_ATTRACTOR:
		sscanf(line, "%d %f %f %f %d %d %d %d %d %d %f %f %f %d",
			&id, &x, &y, &z, &r, &g, &b, &a, &type,
			&flags,
			&effect->attractor.dir.x,
			&effect->attractor.dir.y,
			&effect->attractor.dir.z,
			&probability);
		effect->attractor.type = flags;
#ifdef FIX_BUGS
		effect->attractor.probability = Clamp(probability, 0, 255);
#else
		effect->attractor.probability = probability;
#endif
		break;
	case EFFECT_PED_ATTRACTOR:
		sscanf(line, "%d %f %f %f %d %d %d %d %d %d %f %f %f %f %f %f",
			&id, &x, &y, &z, &r, &g, &b, &a, &type,
			&ptype,
			&effect->pedattr.queueDir.x,
			&effect->pedattr.queueDir.y,
			&effect->pedattr.queueDir.z,
			&effect->pedattr.useDir.x,
			&effect->pedattr.useDir.y,
			&effect->pedattr.useDir.z);
		effect->pedattr.type = ptype;
		break;
	}

	CTxdStore::PopCurrentTxd();
}

#ifdef WANTED_PATHS
void CFileLoader::LoadWantedPathNode(const char* line, int pathID, int pathNode)
{
	float x, y, z;

	sscanf(line, "%f %f %f", &x, &y, &z);

	ThePaths.StoreWantedNode(pathID, pathNode, x, y, z);
}
#endif

#ifdef NEW_VEHICLE_LOADER
void CFileLoader::LoadAnotherVehicleObject(const char* line)
{
	char param[10];
	int id;
	CVehicleModelInfo* mi;

	sscanf(line, "%s", &param);
	if (strcmp(param, "default") == 0) {
		char model[24], txd[24];
		char type[8], handlingId[16], gamename[32], animFile[16], vehclass[12];
		uint32 frequency, comprules;
		int32 level, misc;
		float wheelScale;
		char* p;
		char policeRadio[13];

		sscanf(line, "%s %d %s %s %s %s %s %s %s %d %d %x %d %f %s",
			&param, &id, model, txd,
			type, handlingId, gamename, animFile, vehclass,
			&frequency, &level, &comprules, &misc, &wheelScale, policeRadio);

		mi = CModelInfo::AddVehicleModel(id);
		mi->SetModelName(model);
		mi->SetTexDictionary(txd);
		mi->SetAnimFile(animFile);
		strcpy(mi->m_anotherAnimFileName, animFile);
		mi->m_level = level;
		mi->m_compRules = comprules;

		for (p = gamename; *p; p++)
			if (*p == '_') *p = ' ';
		for (int i = 0; i < 32; i++)
			mi->m_fullGameName[i] = gamename[i];

		if(strcmp(type, "car") == 0){
			mi->m_wheelId = misc;
			mi->m_wheelScale = wheelScale;
			mi->m_vehicleType = VEHICLE_TYPE_CAR;
		}else if(strcmp(type, "boat") == 0){
			mi->m_vehicleType = VEHICLE_TYPE_BOAT;
		}else if(strcmp(type, "train") == 0){
			mi->m_vehicleType = VEHICLE_TYPE_TRAIN;
		}else if(strcmp(type, "heli") == 0){
			mi->m_vehicleType = VEHICLE_TYPE_HELI;
		}else if(strcmp(type, "plane") == 0){
			mi->m_planeLodId = misc;
			mi->m_wheelScale = 1.0f;
			mi->m_vehicleType = VEHICLE_TYPE_PLANE;
		}else if(strcmp(type, "bike") == 0){
			mi->m_bikeSteerAngle = misc;
			mi->m_wheelScale = wheelScale;
			mi->m_vehicleType = VEHICLE_TYPE_BIKE;
		}else
			assert(0);

		if (strcmp(policeRadio, "motorbike") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_MOTOBIKE;
		else if (strcmp(policeRadio, "wagon") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_STATION_WAGON;
		else if (strcmp(policeRadio, "garbagetruck") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_GARBAGE_TRUCK;
		else if (strcmp(policeRadio, "twodoor") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_TUDOOR;
		else if (strcmp(policeRadio, "sedan") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_SEDAN;
		else if (strcmp(policeRadio, "coach") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_COACH;
		else if (strcmp(policeRadio, "hearse") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_HEARSE;
		else if (strcmp(policeRadio, "moped") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_MOPED;
		else if (strcmp(policeRadio, "plane") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_PLANE;
		else if (strcmp(policeRadio, "boat") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_BOAT;
		else if (strcmp(policeRadio, "golfcart") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_GOLF_CART;
		else if (strcmp(policeRadio, "dinghy") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_DINGHY;
		else if (strcmp(policeRadio, "offroad") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_OFFROAD;
		else if (strcmp(policeRadio, "sportscar") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_SPORTS_CAR;
		else if (strcmp(policeRadio, "rig") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_RIG;
		else if (strcmp(policeRadio, "tank") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_TANK;
		else if (strcmp(policeRadio, "bus") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_BUS;
		else if (strcmp(policeRadio, "cruiser") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_CRUISER;
		else if (strcmp(policeRadio, "firetruck") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_FIRE_TRUCK;
		else if (strcmp(policeRadio, "lowrider") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_LOWRIDER;
		else if (strcmp(policeRadio, "van") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_VAN;
		else if (strcmp(policeRadio, "truck") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_TRUCK;
		else if (strcmp(policeRadio, "ambulance") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_AMBULANCE;
		else if (strcmp(policeRadio, "taxi") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_TAXI;
		else if (strcmp(policeRadio, "pickup") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_PICKUP;
		else if (strcmp(policeRadio, "icecream") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_ICE_CREAM_VAN;
		else if (strcmp(policeRadio, "buggy") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_BUGGY;
		else if (strcmp(policeRadio, "helicopter") == 0 || strcmp(policeRadio, "heli") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_HELICOPTER;
		else if (strcmp(policeRadio, "policecar") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_POLICE_CAR;
		else if (strcmp(policeRadio, "swatvan") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_SWAT_VAN;
		else if (strcmp(policeRadio, "speedboat") == 0)
			mi->m_policeRadioIndex = SFX_POLICE_RADIO_SPEEDBOAT;
		else
			mi->m_policeRadioIndex = 0;

		mi->vehicleShadowData.fHeightMultiplier = 1.0f;
		mi->vehicleShadowData.fWidthMultiplier = 1.0f;
		mi->vehicleShadowData.fSizeMultiplier = 1.0f;

		if(strcmp(vehclass, "normal") == 0)
			mi->m_vehicleClass = CCarCtrl::NORMAL;
		else if(strcmp(vehclass, "poorfamily") == 0)
			mi->m_vehicleClass = CCarCtrl::POOR;
		else if(strcmp(vehclass, "richfamily") == 0)
			mi->m_vehicleClass = CCarCtrl::RICH;
		else if(strcmp(vehclass, "executive") == 0)
			mi->m_vehicleClass = CCarCtrl::EXEC;
		else if(strcmp(vehclass, "worker") == 0)
			mi->m_vehicleClass = CCarCtrl::WORKER;
		else if(strcmp(vehclass, "big") == 0)
			mi->m_vehicleClass = CCarCtrl::BIG;
		else if(strcmp(vehclass, "taxi") == 0)
			mi->m_vehicleClass = CCarCtrl::TAXI;
		else if(strcmp(vehclass, "moped") == 0)
			mi->m_vehicleClass = CCarCtrl::MOPED;
		else if(strcmp(vehclass, "motorbike") == 0)
			mi->m_vehicleClass = CCarCtrl::MOTORBIKE;
		else if(strcmp(vehclass, "leisureboat") == 0)
			mi->m_vehicleClass = CCarCtrl::LEISUREBOAT;
		else if(strcmp(vehclass, "workerboat") == 0)
			mi->m_vehicleClass = CCarCtrl::WORKERBOAT;
		else if(strcmp(vehclass, "ignore") == 0) {
			mi->m_vehicleClass = -1;
			return;
		}

		CCarCtrl::AddToCarArray(id, mi->m_vehicleClass);
		mi->m_frequency = frequency;
	} else if (strcmp(param, "handling") == 0) {
		char *start, *end;
		char handlingLine[300];
		char delim[4];
		char* word;
		int field, handlingId;
		int keepGoing;

		strcpy(handlingLine, line);

		field = 0;
		strcpy(delim, " \t");
		for(word = strtok(handlingLine, delim); word; word = strtok(nil, delim)){
			switch(field){
			case  1: 
				mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(atof(word)); 
				mi->handlingData.bBike = mi->m_vehicleType == VEHICLE_TYPE_BIKE ? 1 : 0;
				break;
			case  2: mi->handlingData.fMass = atof(word); break;
			case  3: mi->handlingData.Dimension.x = atof(word); break;
			case  4: mi->handlingData.Dimension.y = atof(word); break;
			case  5: mi->handlingData.Dimension.z = atof(word); break;
			case  6: mi->handlingData.CentreOfMass.x = atof(word); break;
			case  7: mi->handlingData.CentreOfMass.y = atof(word); break;
			case  8: mi->handlingData.CentreOfMass.z = atof(word); break;
			case  9: mi->handlingData.nPercentSubmerged = atoi(word); break;
			case 10: mi->handlingData.fTractionMultiplier = atof(word); break;
			case 11: mi->handlingData.fTractionLoss = atof(word); break;
			case 12: mi->handlingData.fTractionBias = atof(word); break;
			case 13: mi->handlingData.Transmission.nNumberOfGears = atoi(word); break;
			case 14: mi->handlingData.Transmission.fMaxVelocity = atof(word); break;
			case 15: mi->handlingData.Transmission.fEngineAcceleration = atof(word) * 0.4; break;
			case 16: mi->handlingData.Transmission.nDriveType = word[0]; break;
			case 17: mi->handlingData.Transmission.nEngineType = word[0]; break;
			case 18: mi->handlingData.fBrakeDeceleration = atof(word); break;
			case 19: mi->handlingData.fBrakeBias = atof(word); break;
			case 20: mi->handlingData.bABS = !!atoi(word); break;
			case 21: mi->handlingData.fSteeringLock = atof(word); break;
			case 22: mi->handlingData.fSuspensionForceLevel = atof(word); break;
			case 23: mi->handlingData.fSuspensionDampingLevel = atof(word); break;
			case 24: mi->handlingData.fSeatOffsetDistance = atof(word); break;
			case 25: mi->handlingData.fCollisionDamageMultiplier = atof(word); break;
			case 26: mi->handlingData.nMonetaryValue = atoi(word); break;
			case 27: mi->handlingData.fSuspensionUpperLimit = atof(word); break;
			case 28: mi->handlingData.fSuspensionLowerLimit = atof(word); break;
			case 29: mi->handlingData.fSuspensionBias = atof(word); break;
			case 30: mi->handlingData.fSuspensionAntidiveMultiplier = atof(word); break;
			case 31:
				sscanf(word, "%x", &mi->handlingData.Flags);
				mi->handlingData.Transmission.Flags = mi->handlingData.Flags;
				break;
			case 32: mi->handlingData.FrontLights = atoi(word); break;
			case 33: mi->handlingData.RearLights = atoi(word); break;
			}
			field++;
		}
		mod_HandlingManager.ConvertDataToGameUnits(&mi->handlingData);
	} else if (strcmp(param, "handling2") == 0) {
		char *start, *end;
		char handlingLine[201];
		char delim[4];
		char* word;
		int field, handlingId;
		int keepGoing;

		sscanf(line, "%s %d",
			&param, &id);

		mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(id);

		strcpy(handlingLine, line);

		field = 0;
		strcpy(delim, " \t");
		if (mi->handlingData.Flags & HANDLING_IS_BIKE) {
			for (word = strtok(handlingLine, delim); word; word = strtok(nil, delim)) {
				switch (field) {
				case  2: mi->bikeHandlingData.fLeanFwdCOM = atof(word); break;
				case  3: mi->bikeHandlingData.fLeanFwdForce = atof(word); break;
				case  4: mi->bikeHandlingData.fLeanBakCOM = atof(word); break;
				case  5: mi->bikeHandlingData.fLeanBackForce = atof(word); break;
				case  6: mi->bikeHandlingData.fMaxLean = atof(word); break;
				case  7: mi->bikeHandlingData.fFullAnimLean = atof(word); break;
				case  8: mi->bikeHandlingData.fDesLean = atof(word); break;
				case  9: mi->bikeHandlingData.fSpeedSteer = atof(word); break;
				case 10: mi->bikeHandlingData.fSlipSteer = atof(word); break;
				case 11: mi->bikeHandlingData.fNoPlayerCOMz = atof(word); break;
				case 12: mi->bikeHandlingData.fWheelieAng = atof(word); break;
				case 13: mi->bikeHandlingData.fStoppieAng = atof(word); break;
				case 14: mi->bikeHandlingData.fWheelieSteer = atof(word); break;
				case 15: mi->bikeHandlingData.fWheelieStabMult = atof(word); break;
				case 16: mi->bikeHandlingData.fStoppieStabMult = atof(word); break;
				}
				field++;
			}
			mod_HandlingManager.ConvertBikeDataToGameUnits(&mi->bikeHandlingData);
		} else if (mi->handlingData.Flags & HANDLING_IS_HELI || mi->handlingData.Flags & HANDLING_IS_PLANE) {
			for(word = strtok(handlingLine, delim); word; word = strtok(nil, delim)){
				switch(field){
				case  2: mi->flyingHandlingData.fThrust = atof(word); break;
				case  3: mi->flyingHandlingData.fThrustFallOff = atof(word); break;
				case  4: mi->flyingHandlingData.fYaw = atof(word); break;
				case  5: mi->flyingHandlingData.fYawStab = atof(word); break;
				case  6: mi->flyingHandlingData.fSideSlip = atof(word); break;
				case  7: mi->flyingHandlingData.fRoll = atof(word); break;
				case  8: mi->flyingHandlingData.fRollStab = atof(word); break;
				case  9: mi->flyingHandlingData.fPitch = atof(word); break;
				case 10: mi->flyingHandlingData.fPitchStab = atof(word); break;
				case 11: mi->flyingHandlingData.fFormLift = atof(word); break;
				case 12: mi->flyingHandlingData.fAttackLift = atof(word); break;
				case 13: mi->flyingHandlingData.fMoveRes = atof(word); break;
				case 14: mi->flyingHandlingData.vecTurnRes.x = atof(word); break;
				case 15: mi->flyingHandlingData.vecTurnRes.y = atof(word); break;
				case 16: mi->flyingHandlingData.vecTurnRes.z = atof(word); break;
				case 17: mi->flyingHandlingData.vecSpeedRes.x = atof(word); break;
				case 18: mi->flyingHandlingData.vecSpeedRes.y = atof(word); break;
				case 19: mi->flyingHandlingData.vecSpeedRes.z = atof(word); break;
				}
				field++;
			}
		} else if (mi->handlingData.Flags & HANDLING_IS_BOAT) {
			for(word = strtok(handlingLine, delim); word; word = strtok(nil, delim)){
				switch(field){
				case  2: mi->boatHandlingData.fThrustY = atof(word); break;
				case  3: mi->boatHandlingData.fThrustZ = atof(word); break;
				case  4: mi->boatHandlingData.fThrustAppZ = atof(word); break;
				case  5: mi->boatHandlingData.fAqPlaneForce = atof(word); break;
				case  6: mi->boatHandlingData.fAqPlaneLimit = atof(word); break;
				case  7: mi->boatHandlingData.fAqPlaneOffset = atof(word); break;
				case  8: mi->boatHandlingData.fWaveAudioMult = atof(word); break;
				case  9: mi->boatHandlingData.vecMoveRes.x = atof(word); break;
				case 10: mi->boatHandlingData.vecMoveRes.y = atof(word); break;
				case 11: mi->boatHandlingData.vecMoveRes.z = atof(word); break;
				case 12: mi->boatHandlingData.vecTurnRes.x = atof(word); break;
				case 13: mi->boatHandlingData.vecTurnRes.y = atof(word); break;
				case 14: mi->boatHandlingData.vecTurnRes.z = atof(word); break;
				case 15: mi->boatHandlingData.fLook_L_R_BehindCamHeight = atof(word); break;
				}
				field++;
			}
		}
	} else if (strcmp(param, "sounds") == 0) {
		sscanf(line, "%s %d",
			&param, &id);

		mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(id);

		if (mi->handlingData.Flags & HANDLING_IS_BOAT) {
			sscanf(line, "%s %d %d %f %d %f %d",
				&param, &id,
				&mi->boatSoundData.volume, &mi->boatSoundData.volumeModificator, 
				&mi->boatSoundData.frequency, &mi->boatSoundData.frequencyModificator, &mi->boatSoundData.bEngineType);
		} else {
			sscanf(line, "%s %d %d %d %d %d %d %d %d",
				&param, &id,
				&mi->vehicleSampleData.m_nAccelerationSampleIndex, &mi->vehicleSampleData.m_nBank, &mi->vehicleSampleData.m_nHornSample,
				&mi->vehicleSampleData.m_nHornFrequency, &mi->vehicleSampleData.m_nSirenOrAlarmSample,
				&mi->vehicleSampleData.m_nSirenOrAlarmFrequency, &mi->vehicleSampleData.m_bDoorType);
		}
	} else if (strcmp(param, "shadow") == 0) {
		sscanf(line, "%s %d",
			&param, &id);

		mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(id);

		sscanf(line, "%s %d %f %f %f",
			&param, &id,
			&mi->vehicleShadowData.fHeightMultiplier, &mi->vehicleShadowData.fWidthMultiplier, &mi->vehicleShadowData.fSizeMultiplier);
	}
	// loading carcols into VehicleModelInfo.cpp, LoadVehicleColours
}
#endif

void
CFileLoader::LoadScene(const char *filename)
{
	enum {
		NONE,
		INST,
		ZONE,
		CULL,
		OCCL,
		PICK,
		PATH,
#ifdef WANTED_PATHS
		WANTED,
#endif
	};
	char *line;
	int fd;
	int section;
	int pathType, pathIndex;

	section = NONE;
	pathIndex = -1;
	debug("Creating objects from %s...\n", filename);

#ifdef WANTED_PATHS
	int maxWantedPathNodes;
	int currentWantedPathNode;
	int wantedPathID;
#endif

	fd = CFileMgr::OpenFile(filename, "rb");
	assert(fd > 0);
	for(line = CFileLoader::LoadLine(fd); line; line = CFileLoader::LoadLine(fd)){
		if(*line == '\0' || *line == '#')
			continue;

		if(section == NONE){
			if(isLine4(line, 'i','n','s','t')) section = INST;
			else if(isLine4(line, 'z','o','n','e')) section = ZONE;
			else if(isLine4(line, 'c','u','l','l')) section = CULL;
			else if(isLine4(line, 'p','i','c','k')) section = PICK;
			else if(isLine4(line, 'p','a','t','h')) section = PATH;
			else if(isLine4(line, 'o','c','c','l')) section = OCCL;
#ifdef WANTED_PATHS
			else if (isLine4(line, 'w', 'a', 'n', 't', 'e', 'd')) section = WANTED;
#endif
		}else if(isLine3(line, 'e','n','d')){
			section = NONE;
		}else switch(section){
		case INST:
			LoadObjectInstance(line);
			break;
		case ZONE:
			LoadZone(line);
			break;
		case CULL:
			LoadCullZone(line);
			break;
		case OCCL:
			LoadOcclusionVolume(line);
			break;
		case PICK:
			// unused
			LoadPickup(line);
			break;
		case PATH:
			if(pathIndex == -1){
				LoadPathHeader(line, pathType);
				pathIndex = 0;
			}else{
				if(pathType == 0)
					LoadPedPathNode(line, -1, pathIndex);
				else if (pathType == 1)
					LoadCarPathNode(line, -1, pathIndex, false);
				else if (pathType == 2)
					LoadCarPathNode(line, -1, pathIndex, true);
				pathIndex++;
				if(pathIndex == 12)
					pathIndex = -1;
			}
			break;
#ifdef WANTED_PATHS
		case WANTED:
			if (pathIndex == -1) {
				sscanf(line, "%i %i", &wantedPathID, &maxWantedPathNodes);
				currentWantedPathNode = 0;
				pathIndex = 0;
			} else {
				LoadWantedPathNode(line, wantedPathID, currentWantedPathNode);
				currentWantedPathNode++;
				if (currentWantedPathNode == maxWantedPathNodes)
					pathIndex = -1;
			}
#endif
		}
	}
	CFileMgr::CloseFile(fd);

	debug("Finished loading IPL\n");
}

void
CFileLoader::LoadObjectInstance(const char *line)
{
	int id;
	char name[24];
	RwV3d trans, scale, axis;
	float angle;
	CSimpleModelInfo *mi;
	RwMatrix *xform;
	CEntity *entity;
	float area;

	if(sscanf(line, "%d %s %f %f %f %f %f %f %f %f %f %f %f",
	          &id, name, &area,
	          &trans.x, &trans.y, &trans.z,
	          &scale.x, &scale.y, &scale.z,
	          &axis.x, &axis.y, &axis.z, &angle) != 13){
		if(sscanf(line, "%d %s %f %f %f %f %f %f %f %f %f %f",
		          &id, name,
		          &trans.x, &trans.y, &trans.z,
		          &scale.x, &scale.y, &scale.z,
		          &axis.x, &axis.y, &axis.z, &angle) != 12)
			return;
		area = 0;
	}

	mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(id);
	if(mi == nil)
		return;
	assert(mi->IsSimple());

	if(!CStreaming::IsObjectInCdImage(id))
		debug("Not in cdimage %s\n", mi->GetModelName());

	angle = -RADTODEG(2.0f * Acos(angle));
	xform = RwMatrixCreate();
	RwMatrixRotate(xform, &axis, angle, rwCOMBINEREPLACE);
	RwMatrixTranslate(xform, &trans, rwCOMBINEPOSTCONCAT);

	if(mi->GetObjectID() == -1){
		if(ThePaths.IsPathObject(id)){
			entity = new CTreadable;
			ThePaths.RegisterMapObject((CTreadable*)entity);
		}else
			entity = new CBuilding;
		entity->SetModelIndexNoCreate(id);
		entity->GetMatrix() = CMatrix(xform);
		entity->m_level = CTheZones::GetLevelFromPosition(&entity->GetPosition());
		entity->m_area = area;
		if(mi->IsBuilding()){
			if(mi->m_isBigBuilding)
				entity->SetupBigBuilding();
			if(mi->m_isSubway)
				entity->bIsSubway = true;
		}
		if(mi->GetLargestLodDistance() < 2.0f)
			entity->bIsVisible = false;
		CWorld::Add(entity);

		CColModel *col = entity->GetColModel();
		if(col->numSpheres || col->numBoxes || col->numTriangles){
			if(col->level != 0)
				CColStore::GetBoundingBox(col->level).ContainRect(entity->GetBoundRect());
		}else
			entity->bUsesCollision = false;

		if(entity->GetPosition().z + col->boundingBox.min.z < 6.0f)
			entity->bUnderwater = true;
	}else{
		entity = new CDummyObject;
		entity->SetModelIndexNoCreate(id);
		entity->GetMatrix() = CMatrix(xform);
		CWorld::Add(entity);
		if(IsGlass(entity->GetModelIndex()) && !mi->m_isArtistGlass)
			entity->bIsVisible = false;
		entity->m_level = CTheZones::GetLevelFromPosition(&entity->GetPosition());
		entity->m_area = area;
	}

	RwMatrixDestroy(xform);
}

void
CFileLoader::LoadZone(const char *line)
{
	char name[24];
	int type, level;
	float minx, miny, minz;
	float maxx, maxy, maxz;

	if(sscanf(line, "%s %d %f %f %f %f %f %f %d", name, &type, &minx, &miny, &minz, &maxx, &maxy, &maxz, &level) == 9)
		CTheZones::CreateZone(name, (eZoneType)type, minx, miny, minz, maxx, maxy, maxz, (eLevelName)level);
}

void
CFileLoader::LoadCullZone(const char *line)
{
	CVector pos;
	float minx, miny, minz;
	float maxx, maxy, maxz;
	int flags;
	int wantedLevelDrop = 0;

	sscanf(line, "%f %f %f %f %f %f %f %f %f %d %d",
		&pos.x, &pos.y, &pos.z,
		&minx, &miny, &minz,
		&maxx, &maxy, &maxz,
		&flags, &wantedLevelDrop);
	CCullZones::AddCullZone(pos, minx, maxx, miny, maxy, minz, maxz, flags, wantedLevelDrop);
}

// unused
void
CFileLoader::LoadPickup(const char *line)
{
	int id;
	float x, y, z;

	sscanf(line, "%d %f %f %f", &id, &x, &y, &z);
}

void
CFileLoader::LoadOcclusionVolume(const char *line)
{
	float x, y, z;
	float width, length, height;
	float angle;

	sscanf(line, "%f %f %f %f %f %f %f",
		&x, &y, &z,
		&width, &length, &height,
		&angle);
	COcclusion::AddOne(x, y, z + height/2.0f, width, length, height, angle);
}


// unused
void
CFileLoader::ReloadPaths(const char *filename)
{
	enum {
		NONE,
		PATH,
	};
	char *line;
	int section = NONE;
	int id, pathType, pathIndex = -1;
	debug("Reloading paths from %s...\n", filename);

	int fd = CFileMgr::OpenFile(filename, "r");
	assert(fd > 0);
	for (line = CFileLoader::LoadLine(fd); line; line = CFileLoader::LoadLine(fd)) {
		if (*line == '\0' || *line == '#')
			continue;

		if (section == NONE) {
			if (isLine4(line, 'p','a','t','h')) {
				section = PATH;
				ThePaths.AllocatePathFindInfoMem(4500);
			}
		} else if (isLine3(line, 'e','n','d')) {
			section = NONE;
		} else {
			switch (section) {
				case PATH:
					if (pathIndex == -1) {
						id = LoadPathHeader(line, pathType);
						pathIndex = 0;
					} else {
						if(pathType == 0)
							LoadPedPathNode(line, id, pathIndex);
						else if (pathType == 1)
							LoadCarPathNode(line, id, pathIndex, false);
						else if (pathType == 2)
							LoadCarPathNode(line, id, pathIndex, true);
						pathIndex++;
						if (pathIndex == 12)
							pathIndex = -1;
					}
					break;
				default:
					break;
			}
		}
	}
	CFileMgr::CloseFile(fd);
}

void
CFileLoader::ReloadObjectTypes(const char *filename)
{
	enum {
		NONE,
		OBJS,
		TOBJ,
		TWODFX
	};
	char *line;
	int section = NONE;
	CModelInfo::ReInit2dEffects();
	debug("Reloading object types from %s...\n", filename);

	CFileMgr::ChangeDir("\\DATA\\MAPS\\");
	int fd = CFileMgr::OpenFile(filename, "r");
	assert(fd > 0);
	CFileMgr::ChangeDir("\\");
	for (line = CFileLoader::LoadLine(fd); line; line = CFileLoader::LoadLine(fd)) {
		if (*line == '\0' || *line == '#')
			continue;

		if (section == NONE) {
			if (isLine4(line, 'o','b','j','s')) section = OBJS;
			else if (isLine4(line, 't','o','b','j')) section = TOBJ;
			else if (isLine4(line, '2','d','f','x')) section = TWODFX;
		} else if (isLine3(line, 'e','n','d')) {
			section = NONE;
		} else {
			switch (section) {
				case OBJS:
				case TOBJ:
					ReloadObject(line);
					break;
				case TWODFX:
					Load2dEffect(line);
					break;
				default:
					break;
			}
		}
	}
	CFileMgr::CloseFile(fd);
}

void
CFileLoader::ReloadObject(const char *line)
{
	int id, numObjs;
	char model[24], txd[24];
	float dist[3];
	uint32 flags;
	CSimpleModelInfo *mi;

	if(sscanf(line, "%d %s %s %d", &id, model, txd, &numObjs) != 4)
		return;

	switch(numObjs){
	case 1:
		sscanf(line, "%d %s %s %d %f %d",
			&id, model, txd, &numObjs, &dist[0], &flags);
		break;
	case 2:
		sscanf(line, "%d %s %s %d %f %f %d",
			&id, model, txd, &numObjs, &dist[0], &dist[1], &flags);
		break;
	case 3:
		sscanf(line, "%d %s %s %d %f %f %f %d",
			&id, model, txd, &numObjs, &dist[0], &dist[1], &dist[2], &flags);
		break;
	}

	mi = (CSimpleModelInfo*) CModelInfo::GetModelInfo(id);
	if (
#ifdef FIX_BUGS
		mi &&
#endif
	    mi->GetModelType() == MITYPE_SIMPLE && !strcmp(mi->GetModelName(), model) && mi->m_numAtomics == numObjs) {
		mi->SetLodDistances(dist);
		SetModelInfoFlags(mi, flags);
	} else {
		printf("Can't reload %s\n", model);
	}
}

// unused mobile function - crashes
void
CFileLoader::ReLoadScene(const char *filename)
{
	char *line;
	CFileMgr::ChangeDir("\\DATA\\");
	int fd = CFileMgr::OpenFile(filename, "r");
	assert(fd > 0);
	CFileMgr::ChangeDir("\\");

	for (line = CFileLoader::LoadLine(fd); line; line = CFileLoader::LoadLine(fd)) {
		if (*line == '#')
			continue;

		if (strncmp(line, "EXIT", 4) == 0)
			break;

		if (strncmp(line, "IDE", 3) == 0) {
			LoadObjectTypes(line + 4);
		}
	}
	CFileMgr::CloseFile(fd);
}
