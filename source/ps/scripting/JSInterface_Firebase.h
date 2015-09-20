#pragma once
class ScriptInterface;

namespace JSI_Firebase
{
	void FirebaseHTTP(ScriptInterface::CxPrivate* UNUSED(pCxPrivate), std::string method, std::string path, std::string data);
	void RegisterScriptFunctions(ScriptInterface& scriptInterface);
}
