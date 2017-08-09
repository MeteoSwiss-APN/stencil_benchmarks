#pragma once

#include <chrono>
#include <stdexcept>

#include "arguments.h"
#include "result.h"

namespace platform {

    class variant_base {
        using stencil_fptr = void (variant_base::*)();

      public:
        variant_base(const arguments_map &args);
        virtual ~variant_base() {}

        result run(const std::string &kernel, int runs = 20);

        static std::vector<std::string> stencil_list();

        virtual void prerun() = 0;
        virtual void postrun() = 0;

        virtual void copy() = 0;
        virtual void copyi() = 0;
        virtual void copyj() = 0;
        virtual void copyk() = 0;
        virtual void avgi() = 0;
        virtual void avgj() = 0;
        virtual void avgk() = 0;
        virtual void sumi() = 0;
        virtual void sumj() = 0;
        virtual void sumk() = 0;
        virtual void lapij() = 0;

      protected:
        inline int index(int i, int j, int k) const { return i * m_istride + j * m_jstride + k * m_kstride; }

        inline int zero_offset() const { return m_data_offset + index(m_halo, m_halo, m_halo); }

        inline int halo() const { return m_halo; }
        inline int isize() const { return m_isize; }
        inline int jsize() const { return m_jsize; }
        inline int ksize() const { return m_ksize; }
        inline int ilayout() const { return m_ilayout; }
        inline int jlayout() const { return m_jlayout; }
        inline int klayout() const { return m_klayout; }
        inline int istride() const { return m_istride; }
        inline int jstride() const { return m_jstride; }
        inline int kstride() const { return m_kstride; }
        inline int storage_size() const { return m_storage_size; }

        virtual bool verify(const std::string &kernel) const = 0;

        template <class F>
        bool verify_loop(F f) const {
            bool success = true;
#pragma omp parallel for collapse(3) reduction(&& : success)
            for (int k = 0; k < m_ksize; ++k)
                for (int j = 0; j < m_jsize; ++j)
                    for (int i = 0; i < m_isize; ++i)
                        success = success && f(i, j, k);
            return success;
        }

        std::size_t touched_elements(const std::string &kernel) const;
        virtual std::size_t touched_bytes(const std::string &kernel) const = 0;

      private:
        template <class Platform, class ValueType>
        friend class variant;

        int m_halo, m_alignment;
        int m_isize, m_jsize, m_ksize;
        int m_ilayout, m_jlayout, m_klayout;
        int m_istride, m_jstride, m_kstride;
        int m_data_offset, m_storage_size;

        stencil_fptr stencil_function(const std::string &kernel);
    };

} // namespace platform