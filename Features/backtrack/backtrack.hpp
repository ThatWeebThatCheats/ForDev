#pragma once
#include <deque>
#include "../../options.hpp"
#include "../../Helpers/math.hpp"
#include "../../Helpers/utils.hpp"
#include "../../Valve_SDK/csgostructs.hpp"

#define TIME_TO_TICKS( dt )	( ( int )( 0.5f + ( float )( dt ) / Interfaces::GlobalVars->interval_per_tick ) )

struct Incoming_Sequence_Record
{
	Incoming_Sequence_Record(int in_reliable, int out_reliable, int in_sequence, float realtime)
	{
		in_reliable_state = in_reliable;
		out_reliable_state = out_reliable;
		in_sequence_num = in_sequence;

		time = realtime;
	}

	int in_reliable_state;
	int out_reliable_state;
	int in_sequence_num;

	float time;
};

class CBacktrack : public Singleton<CBacktrack>
{
public:
	void AddLatency(INetChannel * net_channel, float latency);
	float GetLerpTime();
	void UpdateIncomingSequences();
	std::deque<Incoming_Sequence_Record> sequence_records;
	int last_incoming_sequence = 0;
	INetChannel* netchan;
};
