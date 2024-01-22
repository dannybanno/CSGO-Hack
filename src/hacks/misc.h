#pragma once

class CUserCmd;
namespace hacks
{
	void RunBunnyHop(CUserCmd* cmd) noexcept;

	bool TriggerBot(CUserCmd* cmd) noexcept;

	void RunAimbot(CUserCmd* cmd) noexcept;

	void RunSpinbot(CUserCmd* cmd) noexcept;
}
