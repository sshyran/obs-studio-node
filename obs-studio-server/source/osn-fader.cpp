// Client module for the OBS Studio node module.
// Copyright(C) 2017 Streamlabs (General Workings Inc)
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#include "osn-fader.hpp"
#include "obs.h"
#include "error.hpp"
#include "utility.hpp"
#include "shared.hpp"
#include "osn-source.hpp"

osn::Fader::Manager& osn::Fader::Manager::GetInstance() {
	static osn::Fader::Manager _inst;
	return _inst;
}

void osn::Fader::Register(ipc::server& srv) {
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("Fader");
	cls->register_function(std::make_shared<ipc::function>("Create", std::vector<ipc::type>{ipc::type::Int32}, Create));
	cls->register_function(std::make_shared<ipc::function>("Destroy", std::vector<ipc::type>{ipc::type::UInt64}, Destroy));
	cls->register_function(std::make_shared<ipc::function>("GetDeziBel", std::vector<ipc::type>{ipc::type::UInt64}, GetDeziBel));
	cls->register_function(std::make_shared<ipc::function>("SetDeziBel", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Float}, SetDeziBel));
	cls->register_function(std::make_shared<ipc::function>("GetDeflection", std::vector<ipc::type>{ipc::type::UInt64}, GetDeflection));
	cls->register_function(std::make_shared<ipc::function>("SetDeflection", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Float}, SetDeflection));
	cls->register_function(std::make_shared<ipc::function>("GetMultiplier", std::vector<ipc::type>{ipc::type::UInt64}, GetMultiplier));
	cls->register_function(std::make_shared<ipc::function>("SetMultiplier", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Float}, SetMultiplier));
	cls->register_function(std::make_shared<ipc::function>("Attach", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64}, Attach));
	cls->register_function(std::make_shared<ipc::function>("Detach", std::vector<ipc::type>{ipc::type::UInt64}, Detach));
	cls->register_function(std::make_shared<ipc::function>("AddCallback", std::vector<ipc::type>{ipc::type::UInt64}, AddCallback));
	cls->register_function(std::make_shared<ipc::function>("RemoveCallback", std::vector<ipc::type>{ipc::type::UInt64}, RemoveCallback));
	srv.register_collection(cls);
}

void osn::Fader::Create(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	obs_fader_type type = (obs_fader_type)args[0].value_union.i32;

	obs_fader_t* fader = obs_fader_create(type);
	if (!fader) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to create Fader."));
		AUTO_DEBUG;
		return;
	}

	auto uid = Manager::GetInstance().allocate(fader);
	if (uid == std::numeric_limits<utility::unique_id::id_t>::max()) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Failed to allocate unique id for Fader."));
		obs_fader_destroy(fader);
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::Fader::Destroy(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	auto uid = args[0].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Fader Reference."));
		AUTO_DEBUG;
		return;
	}

	obs_fader_destroy(fader);
	Manager::GetInstance().free(uid);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::Fader::GetDeziBel(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	auto uid = args[0].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Fader Reference."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_fader_get_db(fader)));
	AUTO_DEBUG;
}

void osn::Fader::SetDeziBel(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	auto uid = args[0].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Fader Reference."));
		AUTO_DEBUG;
		return;
	}

	obs_fader_set_db(fader, args[1].value_union.fp32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_fader_get_db(fader)));
	AUTO_DEBUG;
}

void osn::Fader::GetDeflection(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	auto uid = args[0].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Fader Reference."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_fader_get_deflection(fader)));
	AUTO_DEBUG;
}

void osn::Fader::SetDeflection(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	auto uid = args[0].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Fader Reference."));
		AUTO_DEBUG;
		return;
	}

	obs_fader_set_deflection(fader, args[1].value_union.fp32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_fader_get_deflection(fader)));
	AUTO_DEBUG;
}

void osn::Fader::GetMultiplier(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	auto uid = args[0].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Fader Reference."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_fader_get_mul(fader)));
	AUTO_DEBUG;
}

void osn::Fader::SetMultiplier(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	auto uid = args[0].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Fader Reference."));
		AUTO_DEBUG;
		return;
	}

	obs_fader_set_mul(fader, args[1].value_union.fp32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_fader_get_mul(fader)));
	AUTO_DEBUG;
}

void osn::Fader::Attach(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	auto uid_fader = args[0].value_union.ui64;
	auto uid_source = args[1].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid_fader);
	if (!fader) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Fader Reference."));
		AUTO_DEBUG;
		return;
	}

	auto source = osn::Source::Manager::GetInstance().find(uid_source);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Source Reference."));
		AUTO_DEBUG;
		return;
	}

	if (!obs_fader_attach_source(fader, source)) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Error attaching source."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::Fader::Detach(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	auto uid = args[0].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Fader Reference."));
		AUTO_DEBUG;
		return;
	}

	obs_fader_detach_source(fader);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::Fader::AddCallback(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	//!FIXME!
}

void osn::Fader::RemoveCallback(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	//!FIXME!
}
