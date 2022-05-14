/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_FUNCTIONPOINTERFORSTUBSVISITOR_H
#define UNITTESTBOT_FUNCTIONPOINTERFORSTUBSVISITOR_H

#include "AbstractValueViewVisitor.h"

#include <unordered_map>

namespace visitor {
    class FunctionPointerForStubsVisitor : public AbstractValueViewVisitor {
    public:
        explicit FunctionPointerForStubsVisitor(const types::TypesHandler *typesHandler);


        std::string visit(const tests::Tests &tests);

    protected:
        void visitStruct(const types::Type &type,
                         const string &name,
                         const tests::AbstractValueView *view,
                         const string &access,
                         int depth,
                         bool isConstructor = false) override;

        void visitPointer(const types::Type &type,
                          const string &name,
                          const tests::AbstractValueView *view,
                          const string &access,
                          int depth) override;

        void visitArray(const types::Type &type,
                        const string &name,
                        const tests::AbstractValueView *view,
                        const string &access,
                        size_t size,
                        int depth,
                        bool isConstructor = false) override;

        void visitPrimitive(const types::Type &type,
                            const string &name,
                            const tests::AbstractValueView *view,
                            const string &access,
                            int depth,
                            bool isConstructor = false) override;
    };
}


#endif // UNITTESTBOT_FUNCTIONPOINTERFORSTUBSVISITOR_H
