#include "pch.h"
#include "RateMyTeammates.h"

#include <fstream>
#define REVIEW_FILE ".latest_review"
#define REQUEST_ENDPOINT "https://httpbin.org/anything"

BAKKESMOD_PLUGIN(RateMyTeammates, "write a plugin description here", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void RateMyTeammates::onLoad()
{
	_globalCvarManager = cvarManager;
	LOG("Plugin loaded!");

	gameWrapper->HookEvent("Function Engine.GameEvent_Soccar_TA.EventMatchEnded", [this](std::string eventName)
		{
			this->postReviews();
		});
	// !! Enable debug logging by setting DEBUG_LOG = true in logging.h !!
	//DEBUGLOG("RateMyTeammates debug mode enabled");

	// LOG and DEBUGLOG use fmt format strings https://fmt.dev/latest/index.html
	//DEBUGLOG("1 = {}, 2 = {}, pi = {}, false != {}", "one", 2, 3.14, true);

	//cvarManager->registerNotifier("my_aweseome_notifier", [&](std::vector<std::string> args) {
	//	LOG("Hello notifier!");
	//}, "", 0);

	//auto cvar = cvarManager->registerCvar("template_cvar", "hello-cvar", "just a example of a cvar");
	//auto cvar2 = cvarManager->registerCvar("template_cvar2", "0", "just a example of a cvar with more settings", true, true, -10, true, 10 );

	//cvar.addOnValueChanged([this](std::string cvarName, CVarWrapper newCvar) {
	//	LOG("the cvar with name: {} changed", cvarName);
	//	LOG("the new value is: {}", newCvar.getStringValue());
	//});

	//cvar2.addOnValueChanged(std::bind(&RateMyTeammates::YourPluginMethod, this, _1, _2));

	// enabled decleared in the header
	//enabled = std::make_shared<bool>(false);
	//cvarManager->registerCvar("TEMPLATE_Enabled", "0", "Enable the TEMPLATE plugin", true, true, 0, true, 1).bindTo(enabled);

	//cvarManager->registerNotifier("NOTIFIER", [this](std::vector<std::string> params){FUNCTION();}, "DESCRIPTION", PERMISSION_ALL);
	//cvarManager->registerCvar("CVAR", "DEFAULTVALUE", "DESCRIPTION", true, true, MINVAL, true, MAXVAL);//.bindTo(CVARVARIABLE);
	//gameWrapper->HookEvent("FUNCTIONNAME", std::bind(&TEMPLATE::FUNCTION, this));
	//gameWrapper->HookEventWithCallerPost<ActorWrapper>("FUNCTIONNAME", std::bind(&RateMyTeammates::FUNCTION, this, _1, _2, _3));
	//gameWrapper->RegisterDrawable(bind(&TEMPLATE::Render, this, std::placeholders::_1));


	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", [this](std::string eventName) {
	//	LOG("Your hook got called and the ball went POOF");
	//});
	// You could also use std::bind here
	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&RateMyTeammates::YourPluginMethod, this);
}

void RateMyTeammates::onUnload()
{
	LOG("Plugin unloaded!");
}

void RateMyTeammates::onMatchEnd()
{
	//this->reviewTeammates();
	this->postReviews();
	LOG("Teammates reviewed");
}

void RateMyTeammates::reviewTeammates()
{
	std::ofstream stream(gameWrapper->GetBakkesModPath() / "data" / REVIEW_FILE);
	if (stream.is_open())
	{
		ServerWrapper server = gameWrapper->GetCurrentGameState();
		if (!server) return;

		ArrayWrapper<PriWrapper> pris = server.GetPRIs();
		//std::vector<std::string> playerNames;
		for (PriWrapper pri : pris)
		{
			if (!pri) continue;

			std::string name = pri.GetPlayerName().ToString();
			int playerId = pri.GetPlayerID();

			stream << name << ": " << playerId << "\n";
		}
		stream.close();
	}
	else
	{
		LOG("Unable to open file: " + (std::string)REVIEW_FILE);
	}
}

void RateMyTeammates::postReviews()
{
	ServerWrapper server = gameWrapper->GetCurrentGameState();
	if (!server) return;

	ArrayWrapper<PriWrapper> pris = server.GetPRIs();
	std::vector<std::pair<std::string, int>> playerInfos;
	for (PriWrapper pri : pris)
	{
		if (!pri) continue;

		std::string name = pri.GetPlayerName().ToString();
		int playerId = pri.GetPlayerID();

		playerInfos.push_back(std::pair<std::string, int>(name, playerId));
	}

	std::string playerInfosSerialized = "";
	for (std::pair<std::string, int> playerInfo : playerInfos)
	{
		std::string playerName = playerInfo.first;
		int playerId = playerInfo.second;

		playerInfosSerialized += (playerName + "-" + std::to_string(playerId) + "\n");
	}

	CurlRequest req;
	req.url = REQUEST_ENDPOINT;
	req.body = playerInfosSerialized;

	LOG("Sending reviews...");
	HttpWrapper::SendCurlRequest(req, [this](int code, std::string response) {
		LOG("Response: {} {}", code, response);
	});
}