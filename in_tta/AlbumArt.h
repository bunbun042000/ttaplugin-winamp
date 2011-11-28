#pragma once

//extern "C" {

void Wasabi_Init();
void Wasabi_Quit();
void *Wasabi_Malloc(size_t size_in_bytes);
void Wasabi_Free(void *memory_block);

//}
