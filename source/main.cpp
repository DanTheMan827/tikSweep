/*
  tikSweep
  Copyright (C) 2016 Daniel Radtke (DanTheMan827)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <fstream>
#include <cstdlib>
#include <3ds.h>
#include "title_types.h"

u32 wait_key() {
  while (true) {
    hidScanInput();

    u32 keys = hidKeysDown();
    if (keys > 0) {
      return keys;
    }

    gfxFlushBuffers();
    gspWaitForVBlank();
  }
}

int util_compare_u64(const void* e1, const void* e2) {
  u64 id1 = *(u64*) e1;
  u64 id2 = *(u64*) e2;

  return id1 > id2 ? 1 : id1 < id2 ? -1 : 0;
}

void clean_tickets() {
  Result res = 0;
  u32 ticketCount = 0;
  u32 deletedCount = 0;

  if (R_SUCCEEDED(res = AM_GetTicketCount(&ticketCount))) {
    u64* ticketIDs = (u64*) calloc(ticketCount, sizeof(u64));

    if (ticketIDs != NULL) {
      if (R_SUCCEEDED(res = AM_GetTicketList(&ticketCount, ticketCount, 0, ticketIDs))) {
        qsort(ticketIDs, ticketCount, sizeof(u64), util_compare_u64);
        char cur[34];

        for (u32 i = 0; i < ticketCount && R_SUCCEEDED(res); i++) {
          sprintf(cur,"%016llX", ticketIDs[i]);

          std::string curStr = cur;
          std::string typeCheck = curStr.substr(4,4);
          u64 titleID = ticketIDs[i];
          AM_TitleEntry entry;

          if (
            (
              typeCheck == TITLE_GAMEAPP ||
              typeCheck == TITLE_DEMO ||
              typeCheck == TITLE_PATCH ||
              typeCheck == TITLE_DLC ||
              typeCheck == TITLE_DSIWARE
            ) &&
            (
              !R_SUCCEEDED(AM_GetTitleInfo(MEDIATYPE_SD, 1, &titleID, &entry)) &&
              !R_SUCCEEDED(AM_GetTitleInfo(MEDIATYPE_NAND, 1, &titleID, &entry))
            )
          ) {
            AM_DeleteTicket(titleID);
            deletedCount++;

            printf("Deleted %s \n", cur);
          }
        }
      }
    }
  }

  printf("\nTotal tickets: %lu \n", ticketCount);
  printf("Deleted tickets: %lu", deletedCount);
  printf("\n\nPress any button to exit.");
  wait_key();
}

void action_about(gfxScreen_t screen) {
  PrintConsole infoConsole;
  PrintConsole* currentConsole = consoleSelect(&infoConsole);
  consoleInit(screen, &infoConsole);

  consoleClear();

  printf(CONSOLE_RED "\n tikSweep %s by DanTheMan827\n\n" CONSOLE_RESET, BUILD_VERSION);
  printf("  Remove unused tickets.\n\n\n");
  printf(CONSOLE_YELLOW " Build Info\n\n" CONSOLE_RESET);
  printf("    Date: %s \n\n", BUILD_DATE);
  printf("  Commit: %s \n\n", BUILD_REVISION);

  consoleSelect(currentConsole);
}

int main(int argc, const char* argv[]) {
  gfxInitDefault();
  consoleInit(GFX_TOP, NULL);
  amInit();

  AM_InitializeExternalTitleDatabase(false);

  action_about(GFX_BOTTOM);
  printf("This will remove all unused tickets.");
  printf("\n\nPress the A button to continue.\n\n");

  if (wait_key() == KEY_A) {
    clean_tickets();
  }

  amExit();
  gfxExit();
}
