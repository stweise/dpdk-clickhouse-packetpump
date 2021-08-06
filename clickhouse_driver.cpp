#include <clickhouse/client.h>
#include <iostream>
// The structure defined for a packet is used to encapsule the data for transfer between dpdk main application and clickhouse driver
#include "packet.h"

using namespace clickhouse;

class CH
{
public:
	CH();
	~CH();
	void append(struct packet *);
	void insert();
	void show();
private:
	Client *client;
	std::shared_ptr<ColumnUInt64> pkt_length;
	std::shared_ptr<ColumnUInt64> ip_total_length;
	std::shared_ptr<ColumnIPv6> ip_src_addr;
	std::shared_ptr<ColumnIPv6> ip_dst_addr;
};

CH::CH()
{
	//std::cout << "Constructor" << std::endl;
	/// Initialize client connection.
	client = new Client(ClientOptions().SetHost("localhost"));

	/// Create a table.
	client->Execute("CREATE TABLE IF NOT EXISTS test.packets (pkt_length UInt64, ip_total_length UInt64, ip_src_addr IPv6, ip_dst_addr IPv6) ENGINE = Memory");
	pkt_length = std::make_shared<ColumnUInt64>();
	ip_total_length = std::make_shared<ColumnUInt64>();
	ip_src_addr = std::make_shared<ColumnIPv6>();
	ip_dst_addr = std::make_shared<ColumnIPv6>();
}

CH::~CH()
{
	/* TODO: remove, it is for testing only 
	std::cout << "Destructor" << std::endl;
	// Delete table.
	client->Execute("DROP TABLE test.packets");
	*/
	delete client;
}

void CH::append(struct packet *p)
{
	pkt_length->Append(p->pkt_length);
	ip_total_length->Append(p->ip_total_length);
	ip_src_addr->Append((const std::string&)(p->ip_src_addr));
	ip_dst_addr->Append((const std::string&)(p->ip_dst_addr));
}
void CH::insert()
{
	Block block;

	/*
	auto name = std::make_shared<ColumnString>();
	name->Append("one");
	name->Append("seven");
	*/

	block.AppendColumn("pkt_length", pkt_length);
	block.AppendColumn("ip_total_length", ip_total_length);
	block.AppendColumn("ip_src_addr", ip_src_addr);
	block.AppendColumn("ip_dst_addr", ip_dst_addr);
	//std::cout << "Number of rows in block: " << block.GetRowCount() << std::endl;
	//block.AppendColumn("name", name);

	client->Insert("test.packets", block);

	// delete the old data in our vectors
	pkt_length->Clear();
	ip_total_length->Clear();
	ip_src_addr->Clear();
	ip_dst_addr->Clear();
}

void CH::show()
{

	// Select values inserted in the previous step.
	client->Select("SELECT pkt_length, ip_total_length FROM test.packets", [](const Block & block)
	{
		for(size_t i = 0; i < block.GetRowCount(); ++i)
		{
			std::cout << block[0]->As<ColumnUInt64>()->At(i) << " "
								<< block[1]->As<ColumnUInt64>()->At(i) << " "
			          //<< block[1]->As<ColumnString>()->At(i)
			          << "\n";
		}
	}
	              );
}

CH *ch;

extern "C" void CHconstruct()
{
	ch = new CH();
}

extern "C" void CHdestruct()
{
	delete ch;
}

extern "C" void CHappend(struct packet *p)
{
	ch->append(p);
}
extern "C" void CHinsert()
{
	ch->insert();
}
extern "C" void CHshow()
{
	ch->show();
}
