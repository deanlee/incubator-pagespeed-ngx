// Copyright 2010 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "net/instaweb/rewriter/public/device_properties.h"

#include "pagespeed/kernel/base/gtest.h"
#include "pagespeed/kernel/http/http_names.h"
#include "pagespeed/kernel/http/request_headers.h"
#include "pagespeed/kernel/http/user_agent_matcher.h"
#include "pagespeed/kernel/http/user_agent_matcher_test_base.h"

namespace net_instaweb {

class DevicePropertiesTest: public testing::Test {
 protected:
  DevicePropertiesTest()
      : device_properties_(&user_agent_matcher_) { }

  UserAgentMatcher user_agent_matcher_;
  DeviceProperties device_properties_;
};

TEST_F(DevicePropertiesTest, WebpUserAgentIdentificationNoAccept) {
  // NOTE: the purpose here is *not* to test user_agent_matcher's coverage of
  // webp user agents, just to see that they're properly reflected in
  // device_properties_.
  device_properties_.SetUserAgent(UserAgentMatcherTestBase::kIe7UserAgent);
  EXPECT_FALSE(device_properties_.SupportsWebpInPlace());
  EXPECT_FALSE(device_properties_.SupportsWebpRewrittenUrls());
  EXPECT_FALSE(device_properties_.SupportsWebpLosslessAlpha());

  device_properties_.SetUserAgent(UserAgentMatcherTestBase::kTestingWebp);
  EXPECT_FALSE(device_properties_.SupportsWebpInPlace());
  EXPECT_TRUE(device_properties_.SupportsWebpRewrittenUrls());
  EXPECT_FALSE(device_properties_.SupportsWebpLosslessAlpha());

  device_properties_.SetUserAgent(
      UserAgentMatcherTestBase::kTestingWebpLosslessAlpha);
  EXPECT_FALSE(device_properties_.SupportsWebpInPlace());
  EXPECT_TRUE(device_properties_.SupportsWebpRewrittenUrls());
  EXPECT_TRUE(device_properties_.SupportsWebpLosslessAlpha());
}

// See https://code.google.com/p/modpagespeed/issues/detail?id=978
//
// Microsoft (v-evgena@microsoft.com and tobint@microsoft.com)
// suggests that they are planning to start masquerading IE11 on
// desktop as Chrome, and wants us not to send webp.  They have not
// coughed up what the UA will actually be, however, so we can't do
// this with a blacklist yet.
TEST_F(DevicePropertiesTest, NoWebpForChromeWithoutAcceptHeader) {
  // Android Browser is OK -- we will serve it webp without an accept header.
  // Mobile IE actually *does* masquerade as IE as of August 2014, but
  // it's easy to avoid confusion because the UA includes 'Windows Phone'.
  device_properties_.SetUserAgent(
      UserAgentMatcherTestBase::kAndroidICSUserAgent);
  EXPECT_FALSE(device_properties_.SupportsWebpInPlace());
  EXPECT_TRUE(device_properties_.SupportsWebpRewrittenUrls());
  EXPECT_FALSE(device_properties_.SupportsWebpLosslessAlpha());

  device_properties_.SetUserAgent(
      UserAgentMatcherTestBase::kWindowsPhoneUserAgent);
  EXPECT_FALSE(device_properties_.SupportsWebpInPlace());
  EXPECT_FALSE(device_properties_.SupportsWebpRewrittenUrls());
  EXPECT_FALSE(device_properties_.SupportsWebpLosslessAlpha());

  // However Chrome will generally send us an Accept:image/webp header, whereas
  // other browsers such as (we think) IE11 in the future, will not send
  // such Accept headers.  Unfortunately, Chrome only started sending
  // accept:image/webp at version 25, so version 18 will no longer get webp
  // as of this change.
  device_properties_.SetUserAgent(UserAgentMatcherTestBase::kChrome18UserAgent);
  EXPECT_FALSE(device_properties_.SupportsWebpInPlace());
  EXPECT_FALSE(device_properties_.SupportsWebpRewrittenUrls());
  EXPECT_FALSE(device_properties_.SupportsWebpLosslessAlpha());

  // However, Chrome 25 and 37 will get webp due to the accept header.
  RequestHeaders headers;
  headers.Add(HttpAttributes::kAccept, "image/webp");
  device_properties_.ParseRequestHeaders(headers);

  device_properties_.SetUserAgent(UserAgentMatcherTestBase::kChrome37UserAgent);
  EXPECT_TRUE(device_properties_.SupportsWebpInPlace());
  EXPECT_TRUE(device_properties_.SupportsWebpRewrittenUrls());
  EXPECT_TRUE(device_properties_.SupportsWebpLosslessAlpha());

  device_properties_.SetUserAgent(
      UserAgentMatcherTestBase::kOpera1110UserAgent);
  EXPECT_TRUE(device_properties_.SupportsWebpInPlace());
  EXPECT_TRUE(device_properties_.SupportsWebpRewrittenUrls());
  EXPECT_FALSE(device_properties_.SupportsWebpLosslessAlpha());
}

TEST_F(DevicePropertiesTest, WebpUserAgentIdentificationAccept) {
  RequestHeaders headers;
  headers.Add(HttpAttributes::kAccept, "*/*");
  headers.Add(HttpAttributes::kAccept, "image/webp");
  headers.Add(HttpAttributes::kAccept, "text/html");
  device_properties_.ParseRequestHeaders(headers);

  device_properties_.SetUserAgent(UserAgentMatcherTestBase::kIe7UserAgent);
  EXPECT_TRUE(device_properties_.SupportsWebpInPlace());
  EXPECT_TRUE(device_properties_.SupportsWebpRewrittenUrls());
  EXPECT_FALSE(device_properties_.SupportsWebpLosslessAlpha());

  device_properties_.SetUserAgent(UserAgentMatcherTestBase::kTestingWebp);
  EXPECT_TRUE(device_properties_.SupportsWebpInPlace());
  EXPECT_TRUE(device_properties_.SupportsWebpRewrittenUrls());
  EXPECT_FALSE(device_properties_.SupportsWebpLosslessAlpha());

  device_properties_.SetUserAgent(
      UserAgentMatcherTestBase::kTestingWebpLosslessAlpha);
  EXPECT_TRUE(device_properties_.SupportsWebpInPlace());
  EXPECT_TRUE(device_properties_.SupportsWebpRewrittenUrls());
  EXPECT_TRUE(device_properties_.SupportsWebpLosslessAlpha());
}

}  // namespace net_instaweb