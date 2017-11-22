#include <stdio.h> 
#include <time.h> 
#include <windows.h> 
#include <wincrypt.h> 
  
using namespace std; 
  
static int f3_win_rand() 
{ 
	unsigned short ret_rand=0; 
	HCRYPTPROV hCryptProv = NULL; 
	if(!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0)) 
	{ 
		if(GetLastError() != NTE_BAD_KEYSET) 
		{ 
			printf("A cryptographic service handle could not be acquired.\n"); 
			return -1; 
		} 
		else 
		{ 
			if(!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET)) 
			{ 
				printf("Could not create a new key container.\n"); 
				return -1; 
			} 
		} 
	} 
  
	if(!CryptGenRandom(hCryptProv, sizeof(ret_rand), (BYTE*)&ret_rand)) 
		ret_rand = 0; 
  
	if(hCryptProv) 
		CryptReleaseContext(hCryptProv, 0); 
  
	return (int)ret_rand; 
} 
  
  
  
int main(int, char**) 
{ 
	for(int i=0; i<10000; ++i) 
	{ 
		int v = f3_win_rand(); 
		printf("A cryptographic random: %d\n", v); 
	} 
  
	return 0; 
} 

  
  
int main2(int, char**) 
{ 
	srand((unsigned int)time(NULL)); 
  
	HCRYPTPROV hCryptProv = NULL; 
	BYTE pbData[16]={0}; 
  
	LPCSTR UserName = "MyKeyContainer"; 
	printf("lpcstr = %s\n", UserName); 
	printf("hCryptProv = %ld\n", hCryptProv); 
  
	for(int i = 0; i < sizeof(pbData)/sizeof(pbData[0]); ++i) 
	{ 
		printf("0x%02X, ", pbData[i]); 
	}; 
	printf("\n"); 
	printf("---------------------------------------------\n"); 
  
	if(CryptAcquireContextA(&hCryptProv,			   // handle to the CSP 
							UserName,				  // container name 
							NULL,					  // use the default provider 
							PROV_RSA_FULL,			 // provider type 
							0))						// flag values 
	{ 
		printf("A cryptographic context with the %s key container \n", UserName); 
		printf("has been acquired.\n\n"); 
	} 
	else 
	{ 
		//------------------------------------------------------------------- 
		// An error occurred in acquiring the context. This could mean 
		// that the key container requested does not exist. In this case, 
		// the function can be called again to attempt to create a new key 
		// container. Error codes are defined in Winerror.h. 
		if(GetLastError() == NTE_BAD_KEYSET) 
		{ 
			if(CryptAcquireContextA( 
			&hCryptProv, 
			UserName, 
			NULL, 
			PROV_RSA_FULL, 
			CRYPT_NEWKEYSET)) 
			{ 
				printf("A new key container has been created.\n"); 
			} 
			else 
			{ 
				printf("Could not create a new key container.\n"); 
				exit(1); 
			} 
		} 
		else 
		{ 
			printf("A cryptographic service handle could not be " 
			"acquired.\n"); 
			exit(1); 
		} 
  
	} // End of else. 
	//------------------------------------------------------------------- 
	// A cryptographic context and a key container are available. Perform 
	// any functions that require a cryptographic provider handle. 
  
	//------------------------------------------------------------------- 
	// When the handle is no longer needed, it must be released. 
  
	/*if (CryptReleaseContext(hCryptProv,0)) 
	{ 
	printf("The handle has been released.\n"); 
	} 
	else 
	{ 
	printf("The handle could not be released.\n"); 
	}*/ 
	 
	printf("username = %s\n", UserName); 
	printf("pbdata is now %s\n", pbData); 
  
	if(CryptGenRandom(hCryptProv, 8, pbData)) 
	{ 
		printf("Random sequence generated. \n"); 
	} 
	else 
	{ 
		printf("Error during CryptGenRandom.\n"); 
		printf("now: %ld\n", hCryptProv); 
		exit(1); 
	} 
	 
	for(int i = 0; i < sizeof(pbData)/sizeof(pbData[0]); ++i) 
	{ 
		printf("0x%02X, ", pbData[i]); 
	}; 
	printf("\n"); 
	printf("---------------------------------------------\n"); 
  
	printf("hCryptProv is %ld\n", hCryptProv); 
  
	HCRYPTKEY hKey; 
	ALG_ID ENCRYPT_ALGORITHM = PROV_RSA_FULL; 
	DWORD KEYLENGTH = 3; 
	if(CryptGenKey(hCryptProv, ENCRYPT_ALGORITHM, KEYLENGTH | CRYPT_EXPORTABLE, &hKey)) 
	{ 
		printf("A session key has been created.\n"); 
	} 
	else 
	{ 
		printf("Error during CryptGenKey.\n"); 
		exit(1); 
	} 
	//------------------------------------------------------------------- 
	//  The key created can be exported into a key BLOB that can be 
	//  written to a file. 
	//  ... 
	//  When you have finished using the key, free the resource. 
	printf("hKey = %ld\n", hKey); 
	if(!CryptDestroyKey(hKey)) 
	{ 
		printf("Error during CryptDestroyKey.\n"); 
		exit(1); 
	} 
	printf("hKey = %ld\n", hKey); 
	return 0; 
} 

