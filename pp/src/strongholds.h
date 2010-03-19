class Stronghold
{
	public:
		Stronghold();
		~Stronghold();
		int getID() { return ID; };

	protected:
		ROOM_DATA* heart;
		int ID;
};
