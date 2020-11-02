//
// MIT License
//
// Copyright 2019-2020
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include <filesystem>
#include <map>
#include <string>

#include "apk/apk.h"
#include "apk_parser.h"
#include "binary_xml/binary_xml.h"
#include "utils/log.h"
#include "utils/macros.h"
#include "utils/utils.h"

using namespace ai;

namespace fs = std::filesystem;

namespace {

static constexpr char const *const ANDROID_MANIFEST = "AndroidManifest.xml";

static constexpr auto ANDROID_MANIFEST_TAG_APPLICATION = "application";

static constexpr auto ANDROID_MANIFEST_ATTRIBUTE_DEBUGGABLE = "debuggable";

auto getAndroidManifestAsBinaryXml(std::string const apkPath) -> BinaryXml {
  auto const apkParser = ai::ApkParser(apkPath);
  auto const files = apkParser.getFiles();
  auto const hasAndroidManifest = std::find(files.cbegin(), files.cend(), ANDROID_MANIFEST) != files.end();
  if (!hasAndroidManifest) {
    LOGW("unable to find manifest in [{}]", apkPath);
    throw MissingAndroidManifestException(apkPath);
  }

  auto const contents = apkParser.getFileContents(ANDROID_MANIFEST);
  if (contents.empty()) {
    LOGW("unable to read [{}]", apkPath);
    throw MissingAndroidManifestException(apkPath);
  }

  return BinaryXml(contents);
}

auto getPackageName(std::string const &apkPath) -> std::string {
  auto const apkParser = ai::ApkParser(apkPath);
  auto const androidManifestContents = apkParser.getFileContents(ANDROID_MANIFEST);
  auto const elementAttributes = BinaryXml(androidManifestContents).getElementAttributes(std::vector<std::string>{"manifest"});
  auto const packageName = elementAttributes.find("package");
  if (packageName != elementAttributes.end()) {
    return packageName->second;
  } else {
    return std::string();
  }
}

auto getVersionName(std::string const &apkPath) -> std::string {
  auto const apkParser = ai::ApkParser(apkPath);
  auto const androidManifestContents = apkParser.getFileContents(ANDROID_MANIFEST);
  auto const elementAttributes = BinaryXml(androidManifestContents).getElementAttributes(std::vector<std::string>{"manifest"});
  auto const packageName = elementAttributes.find("versionName");
  if (packageName != elementAttributes.end()) {
    return packageName->second;
  } else {
    return std::string();
  }
}

auto getVersionCode(std::string const &apkPath) -> std::string {
  auto const apkParser = ai::ApkParser(apkPath);
  auto const androidManifestContents = apkParser.getFileContents(ANDROID_MANIFEST);
  auto const elementAttributes = BinaryXml(androidManifestContents).getElementAttributes(std::vector<std::string>{"manifest"});
  auto const packageName = elementAttributes.find("versionCode");
  if (packageName != elementAttributes.end()) {
    return packageName->second;
  } else {
    return std::string();
  }
}

} // namespace

class Apk::ApkImpl final {
public:
  ApkImpl(std::string_view apkPath) : apkPath_(apkPath) {}

  auto isValid() const -> bool {
    try {
      auto const androidManifest = getAndroidManifestAsBinaryXml(apkPath_);
      return androidManifest.hasElement(ANDROID_MANIFEST_TAG_APPLICATION);
    } catch (MissingAndroidManifestException const &e) {
      LOGW("apk is not valid; missing application tag in android manifest");
      return false;
    }
  }

  auto makeDebuggable() const -> void {
    auto const androidManifest = getAndroidManifestAsBinaryXml(apkPath_);
    if (!androidManifest.hasElement(ANDROID_MANIFEST_TAG_APPLICATION)) {
      LOGW("unable to find application tag in android manifest [{}]", apkPath_);
      throw MalformedAndroidManifestException(apkPath_);
    }
    std::vector<std::string> const elementPath = {ANDROID_MANIFEST_TAG_APPLICATION};
    androidManifest.setElementAttribute(elementPath, ANDROID_MANIFEST_ATTRIBUTE_DEBUGGABLE, "true");
  }

  auto isDebuggable() const -> bool {
    auto const androidManifest = getAndroidManifestAsBinaryXml(apkPath_);
    if (!androidManifest.hasElement(ANDROID_MANIFEST_TAG_APPLICATION)) {
      LOGW("unable to find application tag in [{}]", apkPath_);
      throw MalformedAndroidManifestException(apkPath_);
    }
    auto const elementPath = std::vector<std::string>{ANDROID_MANIFEST_TAG_APPLICATION};
    auto const attributes = androidManifest.getElementAttributes(elementPath);
    auto const debuggableAttribute = attributes.find(ANDROID_MANIFEST_ATTRIBUTE_DEBUGGABLE);
    return debuggableAttribute != attributes.end() ? debuggableAttribute->second == "true" : false;
  }

  auto getAndroidManifest() const -> std::string {
    auto const androidManifest = getAndroidManifestAsBinaryXml(apkPath_);
    return androidManifest.toStringXml();
  }

  auto getFiles() const -> std::vector<std::string> {
    auto const apkParser = ai::ApkParser(apkPath_);
    return apkParser.getFiles();
  }

  auto getFileContent(std::string_view filePath) const -> std::vector<std::byte> {
    auto const apkParser = ai::ApkParser(apkPath_);
    return apkParser.getFileContents(filePath);
  }

  auto getProperties() const -> std::map<std::string, std::string> {
    auto const isApkValid = isValid();
    auto properties = std::map<std::string, std::string>{{"valid", isApkValid ? "true" : "false"}};

    if (isValid()) {
      properties.insert({"debuggable", isDebuggable() ? "true" : "false"});
      properties.insert({"manifest", getAndroidManifest()});
      properties.insert({"packageName", getPackageName(apkPath_)});
      properties.insert({"versionCode", getVersionCode(apkPath_)});
      properties.insert({"versionName", getVersionName(apkPath_)});
    }

    return properties;
  }

  auto dump(std::string_view destinationDirectory) const -> void {
    fs::create_directories(destinationDirectory);
    auto const androidManifest = getAndroidManifest();
    fs::path androidManifestFile = fs::path(destinationDirectory) / ANDROID_MANIFEST;
    utils::writeToFile(androidManifestFile, androidManifest);
  }

private:
  std::string const apkPath_;
};

Apk::Apk(std::string_view apkPath) : pimpl_(std::make_unique<Apk::ApkImpl>(apkPath)) {}

Apk::~Apk() = default;

auto Apk::isValid() const -> bool { return pimpl_->isValid(); }

auto Apk::makeDebuggable() const -> void { return pimpl_->makeDebuggable(); }

auto Apk::isDebuggable() const -> bool { return pimpl_->isDebuggable(); }

auto Apk::getAndroidManifest() const -> std::string { return pimpl_->getAndroidManifest(); }

auto Apk::getFiles() const -> std::vector<std::string> { return pimpl_->getFiles(); }

auto Apk::getFileContent(std::string_view filePath) const -> std::vector<std::byte> { return pimpl_->getFileContent(filePath); }

auto Apk::getProperties() const -> std::map<std::string, std::string> { return pimpl_->getProperties(); }

auto Apk::dump(std::string_view destinationDirectory) const -> void { return pimpl_->dump(destinationDirectory); }
