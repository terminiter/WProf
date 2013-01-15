// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jingle/notifier/base/gaia_token_pre_xmpp_auth.h"

#include <algorithm>

#include "base/basictypes.h"
#include "base/logging.h"
#include "talk/base/socketaddress.h"
#include "talk/xmpp/constants.h"
#include "talk/xmpp/saslcookiemechanism.h"

namespace notifier {

namespace {

class GaiaCookieMechanism : public buzz::SaslCookieMechanism {
 public:
  GaiaCookieMechanism(const std::string & mechanism,
                      const std::string & username,
                      const std::string & cookie,
                      const std::string & token_service)
      : buzz::SaslCookieMechanism(
          mechanism, username, cookie, token_service) {}

  virtual ~GaiaCookieMechanism() {}

  virtual buzz::XmlElement* StartSaslAuth() {
    buzz::XmlElement* auth = buzz::SaslCookieMechanism::StartSaslAuth();
    // These attributes are necessary for working with non-gmail gaia
    // accounts.
    const std::string NS_GOOGLE_AUTH_PROTOCOL(
        "http://www.google.com/talk/protocol/auth");
    const buzz::QName QN_GOOGLE_ALLOW_GENERATED_JID_XMPP_LOGIN(
        NS_GOOGLE_AUTH_PROTOCOL, "allow-generated-jid");
    const buzz::QName QN_GOOGLE_AUTH_CLIENT_USES_FULL_BIND_RESULT(
        NS_GOOGLE_AUTH_PROTOCOL, "client-uses-full-bind-result");
    auth->SetAttr(QN_GOOGLE_ALLOW_GENERATED_JID_XMPP_LOGIN, "true");
    auth->SetAttr(QN_GOOGLE_AUTH_CLIENT_USES_FULL_BIND_RESULT, "true");
    return auth;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(GaiaCookieMechanism);
};

}  // namespace

// By default use a Google cookie auth mechanism.
const char GaiaTokenPreXmppAuth::kDefaultAuthMechanism[] = "X-GOOGLE-TOKEN";

GaiaTokenPreXmppAuth::GaiaTokenPreXmppAuth(
    const std::string& username,
    const std::string& token,
    const std::string& token_service,
    const std::string& auth_mechanism)
    : username_(username),
      token_(token),
      token_service_(token_service),
      auth_mechanism_(auth_mechanism) {
  DCHECK(!auth_mechanism_.empty());
}

GaiaTokenPreXmppAuth::~GaiaTokenPreXmppAuth() { }

void GaiaTokenPreXmppAuth::StartPreXmppAuth(
    const buzz::Jid& jid,
    const talk_base::SocketAddress& server,
    const talk_base::CryptString& pass,
    const std::string& auth_mechanism,
    const std::string& auth_token) {
  SignalAuthDone();
}

bool GaiaTokenPreXmppAuth::IsAuthDone() const {
  return true;
}

bool GaiaTokenPreXmppAuth::IsAuthorized() const {
  return true;
}

bool GaiaTokenPreXmppAuth::HadError() const {
  return false;
}

int GaiaTokenPreXmppAuth::GetError() const {
  return 0;
}

buzz::CaptchaChallenge GaiaTokenPreXmppAuth::GetCaptchaChallenge() const {
  return buzz::CaptchaChallenge();
}

std::string GaiaTokenPreXmppAuth::GetAuthToken() const {
  return token_;
}

std::string GaiaTokenPreXmppAuth::GetAuthMechanism() const {
  return auth_mechanism_;
}

std::string GaiaTokenPreXmppAuth::ChooseBestSaslMechanism(
    const std::vector<std::string> & mechanisms, bool encrypted) {
  return (std::find(mechanisms.begin(),
                    mechanisms.end(), auth_mechanism_) !=
          mechanisms.end()) ? auth_mechanism_ : "";
}

buzz::SaslMechanism* GaiaTokenPreXmppAuth::CreateSaslMechanism(
    const std::string& mechanism) {
  if (mechanism == auth_mechanism_)
    return new GaiaCookieMechanism(
        mechanism, username_, token_, token_service_);
  return NULL;
}

}  // namespace notifier