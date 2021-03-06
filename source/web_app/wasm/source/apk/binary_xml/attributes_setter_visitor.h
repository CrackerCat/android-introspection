//
// MIT License
//
// Copyright 2020
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
#ifndef ANDROID_INTROSPECTION_APK_ATTRIBUTES_SETTER_VISITOR_H_
#define ANDROID_INTROSPECTION_APK_ATTRIBUTES_SETTER_VISITOR_H_

#include <string>
#include <string_view>
#include <vector>

#include "binary_xml_visitor.h"

namespace ai {

class AttributesSetterVisitor : public BinaryXmlVisitor {

public:
  AttributesSetterVisitor(std::vector<std::string> const &elementPath, std::string_view attributeName, std::string_view attributeValue);

  ~AttributesSetterVisitor() override = default;

  auto visit(StartXmlTagElement const &element) -> void override;

  auto visit(EndXmlTagElement const &element) -> void override;

  auto visit(CDataTagElement const &element) -> void override;

  auto visit(InvalidXmlTagElement const &element) -> void override;
};

} // namespace ai

#endif /* ANDROID_INTROSPECTION_APK_ATTRIBUTES_SETTER_VISITOR_H_ */
