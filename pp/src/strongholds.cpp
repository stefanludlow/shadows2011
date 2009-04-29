#include "protos.h"
#include "utils.h"
#include "string.h"
#include <sstream>
#include "strongholds.h"

static int strongholdID=2000;

Stronghold::Stronghold()
{
	ID = strongholdID++;
	heart = allocate_room(ID*100);
	heart->name = "The Heart of the Stronghold";
	heart->description = "This room is very blank and non-descript, but is clearly the heart of a stronghold.\n";
	heart->room_flags |= (INDOORS | LAWFUL | SAFE_Q);
}

void do_stronghold(CHAR_DATA * ch, char *argument, int cmd)
{
	if (IS_MORTAL(ch))
	{
		send_to_char("Huh?\n",ch);
		return;
	}
	Stronghold* s = new Stronghold();
	std::ostringstream oss;
	oss << "#1Stronghold created with vnum " << s->getID() << ".#0\n";
	send_to_char((char*)oss.str().c_str(),ch);
}
