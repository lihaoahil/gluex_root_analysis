#ifndef DTreeInterface_h
#define DTreeInterface_h

#include <map>
#include <set>
#include <string>
#include <iostream>
#include <sstream>

#include <TTree.h>
#include <TObject.h>
#include <TClonesArray.h>
#include <TVector2.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TList.h>
#include <TMap.h>

#include <particleType.h>
//WHAT ABOUT COPYING TO A NEW TREE? (AND CUTTING?)
	//https://root.cern.ch/doc/master/copytree3_8C.html

//Set branch status?
	//https://root.cern.ch/doc/master/copytree_8C.html

using namespace std;

//https://halldweb1.jlab.org/wiki/index.php/Analysis_TTreeFormat
class DTreeInterface
{
	//ASSUME: One leaf per branch: No splitting
	//ASSUME: Tree only stores: Fundamental type objects, Fundamental type arrays, TObject's, TClonesArray's

	public:

		/**************************************************************** INITIALIZE ****************************************************************/

		//Constructor
		DTreeInterface(TTree* dTree);

		// Is optional. If needed array size is not specified, it will be set to a default value.
		void Set_InitialArraySize(string locArraySizeBranchName, UInt_t locInitialSize);

		// Is optional. For setting up DParticleCombo. Returns # steps. Keys are step index, particle index.
			//ParticleIndex = -1 for initial, -2 for target, 0+ for final state
		size_t Get_ComboInfo(map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap) const;

		/********************************************************* SPECIFY GET-ENTRY BRANCHES *******************************************************/

		//Specify get entry branches (default is all)
		void Reset_GetEntryBranches(void);
		void Clear_GetEntryBranches(void);
		void Register_GetEntryBranch(string locBranchName);
		void Remove_GetEntryBranch(string locBranchName);

		/************************************************************* GET ENTRY AND FILL ***********************************************************/

		TList* Get_UserInfo(void){return dTree->GetUserInfo();}
		void Get_Entry(Long64_t locEntry);
		void Fill(void){dTree->Fill();};

		/************************************************************ GET BRANCHES AND DATA *********************************************************/

		//GET BRANCH
		TBranch* Get_Branch(string locBranchName) const;

		//GET OBJECT POINTERS
		template <typename DType> DType* Get_Pointer_Fundamental(string locBranchName) const;
		template <typename DType> DType* Get_Pointer_TObject(string locBranchName) const;
		TClonesArray* Get_Pointer_TClonesArray(string locBranchName);

		//GET OBJECTS
		template <typename DType> DType Get_Fundamental(string locBranchName) const;
		template <typename DType> DType Get_Fundamental(string locBranchName, UInt_t locArrayIndex) const;
		template <typename DType> DType Get_TObject(string locBranchName) const;
		template <typename DType> DType Get_TObject(string locBranchName, UInt_t locArrayIndex);

		/******************************************************** CREATE & FILL NEW BRANCHES ********************************************************/

		//CREATE BRANCHES
		template <typename DType> void Create_Branch_Fundamental(string locBranchName);
		template <typename DType> void Create_Branch_NoSplitTObject(string locBranchName);
		template <typename DType> void Create_Branch_FundamentalArray(string locBranchName, string locArraySizeString, unsigned int locInitialSize);
		template <typename DType> void Create_Branch_ClonesArray(string locBranchName, unsigned int locSize);

		//FILL BRANCHES
		template <typename DType> void Fill_Fundamental(string locBranchName, DType locValue);
		template <typename DType> void Fill_Fundamental(string locBranchName, DType locValue, unsigned int locArrayIndex);
		template <typename DType> void Fill_TObject(string locBranchName, DType& locObject, unsigned int locArrayIndex);
		template <typename DType> void Fill_TObject(string locBranchName, DType& locObject);

	private:

		/*********************************************************** SETUP INPUT BRANCHES ***********************************************************/

		//Primary function
		void Set_BranchAddresses(void);

		//Setup by type
		void Set_TObjectBranchAddress(string locBranchName, string locClassName);
		void Set_FundamentalBranchAddress(string locBranchName, string locBranchTitle);
		template <typename DType> void Set_FundamentalBranchAddress_Type(string locBranchName, string locArraySizeString);
		template <typename DType> void Set_TObjectBranchAddress(string locBranchName);
		void Set_ClonesArrayBranchAddress(string locBranchName);

		//Utility Function
		void Parse_BranchTitle(string locBranchTitle, string& locArraySizeString, string& locTypeString);

		/************************************************************** MISCELLANEOUS ***************************************************************/

		DTreeInterface(void); //private default constructor: cannot call

		//For ROOT type string for fundamental data variables
			//Defined in https://root.cern.ch/root/htmldoc/TTree.html
		template<typename DType> struct DROOTTypeString { static const char* GetTypeString() {return "";} }; // Main template class

		/************************************************************* MEMBER VARIABLES *************************************************************/

		TTree* dTree;

		/************************************************************ GET ENTRY CONTROL *************************************************************/

		void Update_GetEntryBranches(void);
		void Increase_ArraySize(string locBranchName, string locBranchType, int locNewArraySize);
		template <typename DType> void Increase_ArraySize(string locBranchName, int locNewArraySize);

		bool dGetEntryBranchesModifiedFlag; //when user changes what branches to get, this will signify that the below maps need to be updated

		//Get Branch names
		set<string> dInputBranches; //only call GetEntry on these!!!!
		set<string> dGetEntryBranches; //user-specified subset of dInputBranches. if empty, get all from dInputBranches. 

		//dGetEntryBranches, broken up into those that are arrays, and those that aren't
		set<string> dArraySizeBranches_GetEntryNeeded;
		set<string> dNonArraySizeBranches_GetEntryNeeded;

		//array size mapping //key is array-size branch name, value is set of branches that need that array size
		map<string, set<string> > dArraySizeToBranchMap_GetEntryNeeded;

		/****************************************************** BRANCH MEMORY AND TYPE MAPPING ******************************************************/

		//For fundamental objects/arrays: memory stored in the branches themselves
		map<string, TClonesArray*> dMemoryMap_ClonesArray;
		map<string, TObject*> dMemoryMap_TObject;
		map<string, string> dFundamentalBranchTypeMap; //key is branch name, value is the root string for the branch type

		map<string, TBranch*> dBranchMap; //all branches

		set<string> dInitedClonesArrayBranches;

		/************************************************************ ARRAY SIZE MAPPING ************************************************************/

		//Branch Fundamental Array Maps
		map<string, UInt_t> dFundamentalArraySizeMap; //keys are the names of the branches that contain sizes, value is the current size //make sure has init value!!
		map<string, set<string> > dArraySizeToBranchMap; //key is array-size branch name, value is set of branches that need that array size
		map<string, string> dBranchToArraySizeMap; //key is branch name, value is the name of the branch that contains the size of its array
};

/********************************************************************* INITIALIZE *********************************************************************/

//Constructor
inline DTreeInterface::DTreeInterface(TTree* dTree) : dTree(dTree), dGetEntryBranchesModifiedFlag(true)
{
	Set_BranchAddresses();
}

inline void DTreeInterface::Set_InitialArraySize(string locArraySizeBranchName, UInt_t locInitialSize)
{
	dFundamentalArraySizeMap[locArraySizeBranchName] = locInitialSize;
}

/************************************************************* SPECIFY GET-ENTRY BRANCHES *************************************************************/

inline void DTreeInterface::Reset_GetEntryBranches(void)
{
	dGetEntryBranches = dInputBranches;
	dGetEntryBranchesModifiedFlag = true;
}

inline void DTreeInterface::Clear_GetEntryBranches(void)
{
	dGetEntryBranches.clear();
	dGetEntryBranchesModifiedFlag = true;
}

inline void DTreeInterface::Register_GetEntryBranch(string locBranchName)
{
	dGetEntryBranches.insert(locBranchName);
	dGetEntryBranchesModifiedFlag = true;
}

inline void DTreeInterface::Remove_GetEntryBranch(string locBranchName)
{
	dGetEntryBranches.insert(locBranchName);
	dGetEntryBranchesModifiedFlag = true;
}

/**************************************************************** SETUP INPUT BRANCHES ****************************************************************/

//SET BRANCH ADDRESSES
template <typename DType> inline void DTreeInterface::Set_FundamentalBranchAddress_Type(string locBranchName, string locArraySizeString)
{
	if(locArraySizeString == "")
	{
		dTree->SetBranchAddress(locBranchName.c_str(), new DType());
		return;
	}

	//creating an array: get the array size
	map<string, UInt_t>::iterator locArraySizeIterator = dFundamentalArraySizeMap.find(locArraySizeString);
	UInt_t locInitialArraySize = (locArraySizeIterator != dFundamentalArraySizeMap.end()) ? locArraySizeIterator->second : 100;

	//Create new array, and register the array size information
	dTree->SetBranchAddress(locBranchName.c_str(), new DType[locInitialArraySize]);
	dArraySizeToBranchMap[locArraySizeString].insert(locBranchName);
	dBranchToArraySizeMap[locBranchName] = locArraySizeString;
}

template <typename DType> inline void DTreeInterface::Set_TObjectBranchAddress(string locBranchName)
{
	dMemoryMap_TObject[locBranchName] = (TObject*)(new DType());
	dTree->SetBranchAddress(locBranchName.c_str(), (DType**)&(dMemoryMap_TObject[locBranchName]));
}

inline void DTreeInterface::Set_ClonesArrayBranchAddress(string locBranchName)
{
	dMemoryMap_ClonesArray[locBranchName] = NULL;
	dTree->SetBranchAddress(locBranchName.c_str(), &(dMemoryMap_ClonesArray[locBranchName]));
}

/**************************************************************** GET BRANCHES AND DATA ***************************************************************/

//GET BRANCH
inline TBranch* DTreeInterface::Get_Branch(string locBranchName) const
{
	map<string, TBranch*>::const_iterator locIterator = dBranchMap.find(locBranchName);
	return ((locIterator != dBranchMap.end()) ? locIterator->second : NULL);
}

//GET POINTERS
template <typename DType> inline DType* DTreeInterface::Get_Pointer_Fundamental(string locBranchName) const
{
	TBranch* locBranch = Get_Branch(locBranchName);
	return ((locBranch != NULL) ? (DType*)locBranch->GetAddress() : NULL);
}

template <typename DType> inline DType* DTreeInterface::Get_Pointer_TObject(string locBranchName) const
{
	TBranch* locBranch = Get_Branch(locBranchName);
	return ((locBranch != NULL) ? *(DType**)locBranch->GetAddress() : NULL);
}

inline TClonesArray* DTreeInterface::Get_Pointer_TClonesArray(string locBranchName)
{
	TBranch* locBranch = Get_Branch(locBranchName);
	if(locBranch == NULL)
		return NULL;

	//For some reason, if this is not called immediately, these branches are somehow invalid
	if(dInitedClonesArrayBranches.find(locBranchName) == dInitedClonesArrayBranches.end())
	{
		locBranch->GetEntry(locBranch->GetReadEntry()); //don't change entry number of branch
		dInitedClonesArrayBranches.insert(locBranchName);
	}
	return *(TClonesArray**)locBranch->GetAddress();
}

//GET OBJECTS
template <typename DType> inline DType DTreeInterface::Get_Fundamental(string locBranchName) const
{
	return *(DType*)Get_Branch(locBranchName)->GetAddress();
}

template <typename DType> inline DType DTreeInterface::Get_Fundamental(string locBranchName, UInt_t locArrayIndex) const
{
	return ((DType*)Get_Branch(locBranchName)->GetAddress())[locArrayIndex];
}

template <typename DType> inline DType DTreeInterface::Get_TObject(string locBranchName) const
{
	return **(DType**)Get_Branch(locBranchName)->GetAddress();
}

template <typename DType> inline DType DTreeInterface::Get_TObject(string locBranchName, UInt_t locArrayIndex)
{
	return *((DType*)Get_Pointer_TClonesArray(locBranchName)->At(locArrayIndex));
}

//INCREASE ARRAY SIZE //For when reading only! Not when writing
template <typename DType> inline void DTreeInterface::Increase_ArraySize(string locBranchName, int locNewArraySize)
{
	//create a new, larger array if the current one is too small
		//DOES NOT copy the old results!  In other words, only call BETWEEN entries, not DURING an entry
	DType* locOldBranchAddress = Get_Pointer_Fundamental<DType>(locBranchName);
	dTree->SetBranchAddress(locBranchName.c_str(), new DType[locNewArraySize]);
	delete[] locOldBranchAddress;
}

/************************************************************* CREATE & FILL NEW BRANCHES *************************************************************/

//CREATE BRANCHES
template <typename DType> inline void DTreeInterface::Create_Branch_Fundamental(string locBranchName)
{
	string locTypeString = DROOTTypeString<DType>::GetTypeString();
	string locTypeName = locBranchName + string("/") + locTypeString;
	dBranchMap[locBranchName] = dTree->Branch(locBranchName.c_str(), new DType(), locTypeName.c_str());
}

template <typename DType> inline void DTreeInterface::Create_Branch_NoSplitTObject(string locBranchName)
{
	dMemoryMap_TObject[locBranchName] = (TObject*)(new DType());
	dBranchMap[locBranchName] = dTree->Branch(locBranchName.c_str(), (DType**)&(dMemoryMap_TObject[locBranchName]), 32000, 0); //0: don't split
}

template <typename DType> inline void DTreeInterface::Create_Branch_FundamentalArray(string locBranchName, string locArraySizeString, unsigned int locInitialSize)
{
	string locTypeString = DROOTTypeString<DType>::GetTypeString();
	string locArrayName = locBranchName + string("[") + locArraySizeString + string("]/") + locTypeString;
	dBranchMap[locBranchName] = dTree->Branch(locBranchName.c_str(), new DType[locInitialSize], locArrayName.c_str());
	dFundamentalArraySizeMap[locBranchName] = locInitialSize;
}

template <typename DType> inline void DTreeInterface::Create_Branch_ClonesArray(string locBranchName, unsigned int locSize)
{
	dMemoryMap_ClonesArray[locBranchName] = new TClonesArray(DType::Class()->GetName(), locSize);
	dBranchMap[locBranchName] = dTree->Branch(locBranchName.c_str(), &(dMemoryMap_ClonesArray[locBranchName]), 32000, 0); //0: don't split
	dInitedClonesArrayBranches.insert(locBranchName);
}

//FILL BRANCHES
template <typename DType> inline void DTreeInterface::Fill_Fundamental(string locBranchName, DType locValue)
{
	*Get_Pointer_Fundamental<DType>(locBranchName) = locValue;
}

template <typename DType> inline void DTreeInterface::Fill_Fundamental(string locBranchName, DType locValue, unsigned int locArrayIndex)
{
	//create a new, larger array if the current one is too small
		//would rather do this in advance, but don't assume that the user has done so!
	unsigned int locCurrentArraySize = dFundamentalArraySizeMap[locBranchName];
	DType* locArray = Get_Pointer_Fundamental<DType>(locBranchName);
	if(locArrayIndex >= locCurrentArraySize)
	{
		DType* locOldArray = locArray;
		dTree->SetBranchAddress(locBranchName.c_str(), new DType[locArrayIndex + 1]);
		locArray = Get_Pointer_Fundamental<DType>(locBranchName);

		//copy the old contents into the new array
		for(unsigned int loc_i = 0; loc_i < locCurrentArraySize; ++loc_i)
			locArray[loc_i] = locOldArray[loc_i];

		delete[] locOldArray;
		dFundamentalArraySizeMap[locBranchName] = locArrayIndex + 1;
	}

	//set the data
	locArray[locArrayIndex] = locValue;
}

template <typename DType> inline void DTreeInterface::Fill_TObject(string locBranchName, DType& locObject, unsigned int locArrayIndex)
{
	TClonesArray* locClonesArray = Get_Pointer_TClonesArray(locBranchName);
	*(DType*)locClonesArray->ConstructedAt(locArrayIndex) = locObject;
}

template <typename DType> inline void DTreeInterface::Fill_TObject(string locBranchName, DType& locObject)
{
	*Get_Pointer_TObject<DType>(locBranchName) = locObject;
}

/************************************************************** TEMPLATE SPECIALIZATIONS **************************************************************/

template<> struct DTreeInterface::DROOTTypeString<const char*> { static const char* GetTypeString() {return "C";} };
template<> struct DTreeInterface::DROOTTypeString<Char_t> { static const char* GetTypeString() {return "B";} };
template<> struct DTreeInterface::DROOTTypeString<UChar_t> { static const char* GetTypeString() {return "b";} };
template<> struct DTreeInterface::DROOTTypeString<Short_t> { static const char* GetTypeString() {return "S";} };
template<> struct DTreeInterface::DROOTTypeString<UShort_t> { static const char* GetTypeString() {return "s";} };
template<> struct DTreeInterface::DROOTTypeString<Int_t> { static const char* GetTypeString() {return "I";} };
template<> struct DTreeInterface::DROOTTypeString<UInt_t> { static const char* GetTypeString() {return "i";} };
template<> struct DTreeInterface::DROOTTypeString<Float_t> { static const char* GetTypeString() {return "F";} };
template<> struct DTreeInterface::DROOTTypeString<Double_t> { static const char* GetTypeString() {return "D";} };
template<> struct DTreeInterface::DROOTTypeString<Long64_t> { static const char* GetTypeString() {return "L";} };
template<> struct DTreeInterface::DROOTTypeString<ULong64_t> { static const char* GetTypeString() {return "l";} };
template<> struct DTreeInterface::DROOTTypeString<Bool_t> { static const char* GetTypeString() {return "O";} };

#endif //DTreeInterface_h