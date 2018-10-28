//--------------------------------------------------
//  Fichier: GA.h
//
//  Copyright (C) 2018 Alain Lanthier, septembre 2018
//  License: MIT License 
//--------------------------------------------------
#pragma once

#include "MatriceUtil.h"
#include "Algorithm.h"

using BINAIRO_TEST_TYPE = int;
// 4x4
//static std::vector<BINAIRO_TEST_TYPE> binairo_initial  = { 1,0,-1,0, -1,-1,1,-1, -1,-1,0,0, -1,-1,-1,-1 };
//static std::vector<BINAIRO_TEST_TYPE> binairo_solution = { 1,0, 1,0,  0, 0,1, 1,  1, 1,0,0,  0, 1, 0, 1 };
// 8x8
//static std::vector<BINAIRO_TEST_TYPE> binairo_initial  = {-1,-1,-1,-1,-1,-1,-1, 0, -1, 0, 0,-1,-1, 1,-1,-1,  -1, 0,-1,-1,-1, 1,-1, 0,  -1,-1, 1,-1,-1,-1,-1,-1,
//                                                           0, 0,-1, 1,-1,-1, 1,-1, -1,-1,-1,-1, 1,-1,-1,-1,   1, 1,-1,-1,-1, 0,-1, 1,  -1, 1,-1,-1,-1,-1,-1, 1};
//static std::vector<BINAIRO_TEST_TYPE> binairo_solution = { 0,1,1,0,1,0,1,0,  1,0,0,1,0,1,0,1,  1,0,0,1,0,1,1,0, 0,1,1,0,1,0,0,1,
//                                                           0,0,1,1,0,1,1,0,  1,0,0,1,1,0,1,0,  1,1,0,0,1,0,0,1, 0,1,1,0,0,1,0,1};
static std::vector<BINAIRO_TEST_TYPE> binairo_initial;
void make_binairo()
{
    // hard.txt
    std::string s =
        std::string("*1*1*0**1*") +
        std::string("0*0*******") +
        std::string("******11**") +
        std::string("**1**0****") +
        std::string("0*********") +
        std::string("*******00*") +
        std::string("1******1*1") +
        std::string("**0***0***") +
        std::string("******0*1*") +
        std::string("****0**0**");

        //*1*1*0**1*
        //0*0*******
        //******11**
        //**1**0****
        //0*********
        //*******00*
        //1******1*1
        //**0***0***
        //******0*1*
        //****0**0**

        //0   1   0   1   1   0   0   1   1   0   358
        //0   1   0   1   0   1   0   0   1   1   339
        //1   0   1   0   1   0   1   1   0   0   684
        //1   0   1   1   0   0   1   0   0   1   713
        //0   1   0   0   1   1   0   1   1   0   310
        //0   1   1   0   0   1   1   0   0   1   409
        //1   0   0   1   0   0   1   1   0   1   589
        //1   0   0   1   1   0   0   1   1   0   614
        //0   1   1   0   1   1   0   0   1   0   434
        //1   0   1   0   0   1   1   0   0   1   665
        //205 818 211 844 678 307 217 684 806 345

    binairo_initial = std::vector<BINAIRO_TEST_TYPE>(100);
    for (size_t i = 0; i < 10; i++)
    {
        for (size_t j = 0; j < 10; j++)
        {
            if (s.at(10 * i + j) == '*') binairo_initial[10 * i + j] = -1;
            else if (s.at(10 * i + j) == '0') binairo_initial[10 * i + j] = 0;
            else if (s.at(10 * i + j) == '1') binairo_initial[10 * i + j] = 1;
        }
    }
}

template <typename T>
void FixedParameterBinairo(galgo::Population<T>& x, int k)
{
    std::vector<galgo::CHR<T>>& np = x.get_newpop();

    {
        for (int j = 0; j < x.ga_algo()->force_value_flag.size(); j++)
        {
            if (x.ga_algo()->force_value_flag[j])
            {
                np[k]->initGene(j, x.ga_algo()->force_value[j]);
                if (np[k]->get_value(j) != x.ga_algo()->force_value[j])
                {
                    std::cout << "ERROR - Invalid decode/encode desired_value:" << x.ga_algo()->force_value[j] << " set_value: " << np[k]->get_value(j) << "\n";
                }
            }
        }

        size_t n = (int)pow((double)np[k]->nbgene(), 0.50);
        MatriceUtil<T> mat(n, n);

        int v;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                v = np[k]->get_value((int)n*i + j);
                mat.set(i, j, v);

                if ((v != binairo_initial[n*i + j]) && ((binairo_initial[n*i + j] == 0) || (binairo_initial[n*i + j] == 1)))
                {
                    // Put fixed parameter back into matrice
                    mat.set(i, j, binairo_initial[n*i + j]);
                }
            }
        }

        // Put forced parameter in matrice
        bool is_valid;
        if (try_resolve_binairio(mat, is_valid) == true)
        {
            // SOLVED
        }

        //if (is_valid == true)
        {
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    np[k]->initGene((int)n*i + j, mat[i][j]);
                }
            }
        }

    }
}

template <typename T> class BinairoObjective
{
public:
    static std::vector<double> Objective(const std::vector<T>& x)
    {
        size_t n = (int)pow((double)x.size(), 0.50);
        MatriceUtil<T> mat(n, n);

        bool mismatch = false;
        for (size_t i = 0; i < n; i++)
        {
            for (size_t j = 0; j < n; j++)
            {
                mat.set(i, j, x[n*i + j]);

                if ((x[n*i + j] != binairo_initial[n*i + j]) && ((binairo_initial[n*i + j] == 0) || (binairo_initial[n*i + j] == 1)))
                {
                    // Put fixed parameter back into matrice
                    mat.set(i, j, binairo_initial[n*i + j]);
                    //mismatch = true;
                }
            }
        }

        // Put forced parameter in matrice
        //bool is_valid;
        //if (try_resolve_binairio(mat, is_valid) == true)
        //{
        //    // SOLVED
        //}


        int cnt_0 = mat.count(0);
        int cnt_1 = mat.count(1);
        int cnt_m1 = mat.count(-1);
        int cnt_other = mat.count(2) + mat.count(-2);

        int cnt_illegal_row = 0;
        int cnt_illegal_col = 0;
        int cnt_illegal_sequ = 0;
        int cnt_illegal_rowcol = 0;
        for (size_t i = 0; i < mat.size_row(); i++)
        {
            if (mat.count_row(i, 0) >(int) (mat.size_col() / 2))
                cnt_illegal_row++;

            if (mat.count_row(i, 1) > (int)(mat.size_col() / 2))
                cnt_illegal_row++;

            if (mat.row_max_sequence(i, 0) > 2)
                cnt_illegal_sequ++;

            if (mat.row_max_sequence(i, 1) > 2)
                cnt_illegal_sequ++;

            for (size_t j = 0; j < mat.size_row(); j++)
            {
                if (i < j)
                {
                    if (mat.row_same(i, j) == true)
                        cnt_illegal_rowcol++;
                }
            }
        }

        for (size_t i = 0; i < mat.size_col(); i++)
        {
            if (mat.count_col(i, 0) >(int) (mat.size_row() / 2))
                cnt_illegal_col++;

            if (mat.count_col(i, 1) > (int)(mat.size_row() / 2))
                cnt_illegal_col++;

            if (mat.col_max_sequence(i, 0) > 2)
                cnt_illegal_sequ++;

            if (mat.col_max_sequence(i, 1) > 2)
                cnt_illegal_sequ++;

            for (size_t j = 0; j < mat.size_col(); j++)
            {
                if (i < j)
                {
                    if (mat.col_same(i, j) == true)
                        cnt_illegal_rowcol++;
                }
            }
        }

        double penality = 0.0;
        if (mismatch == true)  penality += 200.0;
        penality += std::fabs((double)cnt_0 - (n * n / 2));
        penality += std::fabs((double)cnt_1 - (n * n / 2));
        penality += std::fabs((double)cnt_m1 - (0));
        penality += std::fabs((double)cnt_other - (0));
        penality += (double)cnt_illegal_row;
        penality += (double)cnt_illegal_col;
        penality += (double)cnt_illegal_sequ;
        penality += (double)cnt_illegal_rowcol;

        double obj = -penality;
        return { obj };
    }
};

void test_ga_binairo()
{
    {
        galgo::MutationInfo<BINAIRO_TEST_TYPE> mutinfo;     // Changes mutation info as desired
        mutinfo._sigma = 1.0;
        mutinfo._sigma_lowest = 0.01;
        mutinfo._ratio_boundary = 0.10;
        mutinfo._type = galgo::MutationType::MutationSPM;

        const int       POPUL = 200;
        const int       N = 200000;
        const double    MUTRATE = 0.05;
        const int       NBIT = 2;

        make_binairo();

        std::cout << std::endl;
        std::cout << "BINAIRO grid NxN";
        int k = 0;
        const int NBinairo = 10;
        BINAIRO_TEST_TYPE low = -1;
        BINAIRO_TEST_TYPE high = 1;
        std::vector<BINAIRO_TEST_TYPE> vlow(NBinairo * NBinairo);
        std::vector<BINAIRO_TEST_TYPE> vhigh(NBinairo * NBinairo);
        std::vector<BINAIRO_TEST_TYPE> vinit(NBinairo * NBinairo);
        for (size_t i = 0; i < NBinairo * NBinairo; i++)
        {
            vlow[i] = low;
            vhigh[i] = high;
            vinit[i] = binairo_initial[i];
        }

        MatriceUtil<BINAIRO_TEST_TYPE> mat(NBinairo, NBinairo);
        bool mismatch = false;
        std::vector<bool> force_value_flag(NBinairo * NBinairo);
        std::vector<BINAIRO_TEST_TYPE> force_value(NBinairo * NBinairo);
        for (size_t i = 0; i < NBinairo; i++)
        {
            for (size_t j = 0; j < NBinairo; j++)
            {
                mat.set(i, j, binairo_initial[NBinairo*i + j]);
                force_value_flag[NBinairo*i + j] = false;
                force_value[NBinairo*i + j] = -1;
                if (binairo_initial[NBinairo*i + j] != -1)
                {
                    force_value_flag[NBinairo*i + j] = true;
                    force_value[NBinairo*i + j] = binairo_initial[NBinairo*i + j];
                }
            }
        }
        display_binairio<BINAIRO_TEST_TYPE>(mat, false);

        galgo::GeneticAlgorithmN<BINAIRO_TEST_TYPE, NBIT> ga(BinairoObjective<BINAIRO_TEST_TYPE>::Objective, POPUL, N, true, mutinfo, vlow, vhigh, vinit);

        ga.mutrate = MUTRATE;
        ga.Selection = RWS;
        ga.CrossOver = P1XO;
        ga.genstep = 50;
        ga.precision = 2;

        ga.force_value_flag = force_value_flag;
        ga.force_value = force_value;
        ga.FixedValue = FixedParameterBinairo;  // nullptr;

        ga.run();

        //const galgo::CHR<BINAIRO_TEST_TYPE> bestParam = ga.result(); //std::shared_ptr<Chromosome<T>>;
        //std::vector<BINAIRO_TEST_TYPE> v = bestParam->getParam();

        system("pause");
    }
}