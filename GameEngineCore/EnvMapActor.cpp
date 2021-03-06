#include "EnvMapActor.h"
#include "RenderContext.h"
#include "Engine.h"

namespace GameEngine
{
	void EnvMapActor::OnLoad()
	{
		envMapId = Engine::Instance()->GetRenderer()->GetSharedResource()->AllocEnvMap();
	}
}