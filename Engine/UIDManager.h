#include <set>

template <typename T>
class UniqueIDManager
{
public:
	T Issue()
	{
		const T startID = nextID;
		do
		{
			if( ids.find( nextID ) == ids.end() )
			{
				ids.insert( nextID );
				return nextID++;
			}
			nextID++;
		} while( nextID != startID );

		throw std::runtime_error( "No available ID left for generation in UniqueIDMangager::Issue()!" );
	}
	void Release( T id )
	{
		ids.erase( id );
	}
private:
	T nextID = T( 0u );
	std::set<T> ids;
};