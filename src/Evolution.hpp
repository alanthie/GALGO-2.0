//=================================================================================================
//                  Copyright (C) 2018 Alain Lanthier - All Rights Reserved  
//                  License: MIT License    See LICENSE.md for the full license.
//                  Original code 2017 Olivier Mallet (MIT License)              
//=================================================================================================

#ifndef EVOLUTION_HPP
#define EVOLUTION_HPP

// In this header, the user can define his own selection, cross-over, mutation and adaptation to 
// constraint(s) methods by respecting the function declaration template

//=================================================================================================

// SELECTION METHODS

/*-------------------------------------------------------------------------------------------------*/

// proportional roulette wheel selection
template <typename T>
void RWS(galgo::Population<T>& x)
{
   // adjusting all fitness to positive values
   x.adjustFitness();

   // computing fitness sum
   double fitsum = x.getSumFitness();

   // selecting mating population
   for (int i = 0, end = x.matsize(); i < end; ++i)
   {
       int j = 0;
       if (fitsum > 0.0)
       {
            // generating a random fitness sum in [0,fitsum)
            double fsum = galgo::uniform<double>(0.0, fitsum);

            while (fsum >= 0.0)
            {
                 #ifndef NDEBUG
                 if (j == x.popsize()) {
                    throw std::invalid_argument("Error: in RWS(galgo::Population<T>&) index j cannot be equal to population size.");
                 }
                 #endif
                 fsum -= x(j)->fitness;
                 j++;
            }
      }
      else
      {
          j = 1;
      }

      // selecting element
      x.select(j - 1);
   }
}

/*-------------------------------------------------------------------------------------------------*/

// stochastic universal sampling selection
template <typename T>
void SUS(galgo::Population<T>& x)
{
   // adjusting all fitness to positive values
   x.adjustFitness();

   // computing fitness sum
   double fitsum = x.getSumFitness();

   int matsize = x.matsize();

   // computing interval size
   double dist = fitsum / matsize;

   // initializing pointer
   double ptr = galgo::uniform<double>(0.0, dist);
   
   // selecting mating population
   for (int i = 0; i < matsize; ++i) {
   
      int j = 0;
      double fsum = 0;
      
      while (fsum <= ptr) {
         #ifndef NDEBUG
         if (j == x.popsize()) {
            throw std::invalid_argument("Error: in SUS(galgo::Population<T>&) index j cannot be equal to population size.");
         }
         #endif
         fsum += x(j)->fitness;
         j++;
      }

      // selecting element
      x.select(j - 1);

      // incrementing pointer
      ptr += dist;
   }
}

/*-------------------------------------------------------------------------------------------------*/

// classic linear rank-based selection
template <typename T>
void RNK(galgo::Population<T>& x)
{
   int popsize = x.popsize();
   static std::vector<int> rank(popsize);
   static int ranksum;

   // this will only be run at the first generation
   if (x.nogen() == 1) {
      int n = popsize + 1;
      // generating ranks from highest to lowest
      std::generate_n(rank.begin(), popsize, [&n]()->int{return --n;});
      // computing sum of ranks
      ranksum = int (.5 * popsize * (popsize + 1));
   }

   // selecting mating population
   for (int i = 0, end = x.matsize(); i < end; ++i) {
      // generating a random rank sum in [1,ranksum)
      int rsum = galgo::uniform<int>(1, ranksum);

      int j = 0;
      while (rsum > 0) {
         #ifndef NDEBUG
         if (j == popsize) {
            throw std::invalid_argument("Error: in RNK(galgo::Population<T>&) index j cannot be equal to population size.");
         }
         #endif
         rsum -= rank[j];
         j++;
      }
      // selecting element
      x.select(j - 1);
   }
}

/*-------------------------------------------------------------------------------------------------*/

// linear rank-based selection with selective pressure
template <typename T>
void RSP(galgo::Population<T>& x)
{
   int popsize = x.popsize();
   static std::vector<double> rank(popsize);
   static double ranksum;

   // this will only be run at the first generation
   if (x.nogen() == 1) {
      // initializing ranksum
      ranksum = 0.0;
      // generating ranks from highest to lowest
      for (int i = 0; i < popsize; ++i) {
         rank[i] =  2 - x.SP() + 2 * (x.SP() - 1) * (popsize - i) / popsize;
         ranksum += rank[i];
      }      
   }

   // selecting mating population
   for (int i = 0, end = x.matsize(); i < end; ++i)
   {
      // generating a random rank sum in [0,ranksum)
      double rsum = galgo::uniform<double>(0.0, ranksum);

      int j = 0;
      while (rsum >= 0.0) {
         #ifndef NDEBUG
         if (j == popsize) {
            throw std::invalid_argument("Error: in RSP(galgo::Population<T>&) index j cannot be equal to population size.");
         }
         #endif
         rsum -= rank[j];
         j++;
      }
      // selecting element
      x.select(j - 1);
   }
}

/*-------------------------------------------------------------------------------------------------*/

// tournament selection
template <typename T>
void TNT(galgo::Population<T>& x)
{
   int popsize = x.popsize();
   int tntsize = x.tntsize();

   // selecting mating population
   for (int i = 0, end = x.matsize(); i < end; ++i) 
   {
      // selecting randomly a first element
      int bestIdx = galgo::uniform<int>(0, popsize);
      double bestFit = x(bestIdx)->fitness;
   
      // starting tournament
      for (int j = 1; j < tntsize; ++j) {
   
         int idx = galgo::uniform<int>(0, popsize);
         double fit = x(idx)->fitness;
      
         if (fit > bestFit) {
            bestFit = fit;
            bestIdx = idx;
         }
      }

      // selecting element
      x.select(bestIdx);
   }
}

/*-------------------------------------------------------------------------------------------------*/

// transform ranking selection
template <typename T>
void TRS(galgo::Population<T>& x)
{
   static double c;
   // (re)initializing when running new GA
   if (x.nogen() == 1) {  
      c =  0.2;
   }
   int popsize = x.popsize();

   // generating a random set of popsize values on [0,1)
   std::vector<double> r(popsize);
   std::for_each(r.begin(),r.end(),[](double& z)->double{return galgo::proba(galgo::rng);});

   // sorting them from highest to lowest
   std::sort(r.begin(),r.end(),[](double z1, double z2)->bool{return z1 > z2;});
   // transforming population fitness
   auto it = x.begin();
   std::for_each(r.begin(),r.end(),[&it,popsize](double z)->void{(*it)->fitness = ceil((popsize - popsize*exp(-c*z))/(1 - exp(-c))); it++;});

   // updating c for next generation
   c = c + 0.1; // arithmetic transition
   //c = c * 1.1; // geometric transition
   // computing fitness sum
   double fitsum = x.getSumFitness();

   // selecting mating population
   for (int i = 0, end = x.matsize(); i < end; ++i)
   {
      int j = 0;
      if (fitsum > 0.0)
      {
          // generating a random fitness sum in [0,fitsum)
          double fsum = galgo::uniform<double>(0, fitsum);
          while (fsum >= 0) 
          {
             #ifndef NDEBUG
             if (j == popsize) 
             {
                throw std::invalid_argument("Error: in TRS(galgo::Population<T>&) index j cannot be equal to population size.");
             }
             #endif
             fsum -= x(j)->fitness;
             j++;
          }
      }
      else
      {
          j = 1;
      }

      // selecting element
      x.select(j - 1);
   }
}

/*-------------------------------------------------------------------------------------------------*/

// CROSS-OVER METHODS

/*-------------------------------------------------------------------------------------------------*/
template <typename T>
void RealValuedSimpleArithmeticRecombination(const galgo::Population<T>& x, galgo::CHR<T>& chr1, galgo::CHR<T>& chr2)
{
    // choosing randomly 2 chromosomes from mating population
    int idx1 = galgo::uniform<int>(0, x.matsize());
    int idx2 = galgo::uniform<int>(0, x.matsize());

    // choosing randomly a position for cross-over
    int pos = galgo::uniform<int>(0, chr1->nbgene());

    double r = chr1->recombination_ratio();
    // *x[idx1] is operator[](int pos) is access element in mating population at position pos
    const galgo::Chromosome<T>& chrmat1 = *x[idx1];
    const galgo::Chromosome<T>& chrmat2 = *x[idx2];

    for (int i = 0; i < pos; i++)
    {
        chr1->initGene(i, chrmat1.get_value(i));
    }
    for (int i = pos; i < chr1->nbgene(); i++)
    {
        chr1->initGene(i, (T)(r * chrmat2.get_value(i) + (1.0 - r) * chrmat1.get_value(i)));
    }

    for (int i = 0; i < pos; i++)
    {
        chr2->initGene(i, chrmat2.get_value(i));
    }
    for (int i = pos; i < chr1->nbgene(); i++)
    {
        chr2->initGene(i, (T)(r * chrmat1.get_value(i) + (1.0 - r) * chrmat2.get_value(i)));
    }

    // Transmit sigma
    for (int i = 0; i < chr1->nbgene(); i++)
    {
        chr1->sigma_update(i, 0.5*(chrmat1.get_sigma(i) + chrmat2.get_sigma(i)));
    }
    for (int i = 0; i < chr2->nbgene(); i++)
    {
        chr2->sigma_update(i, 0.5*(chrmat1.get_sigma(i) + chrmat2.get_sigma(i)));
    }
}

template <typename T>
void RealValuedSingleArithmeticRecombination(const galgo::Population<T>& x, galgo::CHR<T>& chr1, galgo::CHR<T>& chr2)
{
    // choosing randomly 2 chromosomes from mating population
    int idx1 = galgo::uniform<int>(0, x.matsize());
    int idx2 = galgo::uniform<int>(0, x.matsize());

    // choosing randomly a position for cross-over
    int pos = galgo::uniform<int>(0, chr1->nbgene());

    double r = chr1->recombination_ratio();
    const galgo::Chromosome<T>& chrmat1 = *x[idx1];
    const galgo::Chromosome<T>& chrmat2 = *x[idx2];

    for (int i = 0; i < chr1->nbgene(); i++)
    {
        chr1->initGene(i, chrmat1.get_value(i));
    }
    chr1->initGene(pos, (T)(r * chrmat2.get_value(pos) + (1.0 - r) * chrmat1.get_value(pos)));

    for (int i = 0; i < chr2->nbgene(); i++)
    {
        chr2->initGene(i, chrmat2.get_value(i));
    }
    chr2->initGene(pos, (T)(r * chrmat1.get_value(pos) + (1.0 - r) * chrmat2.get_value(pos)));

    // Transmit sigma
    for (int i = 0; i < chr1->nbgene(); i++)
    {
        chr1->sigma_update(i, (0.5*(chrmat1.get_sigma(i) + chrmat2.get_sigma(i))));
    }
    for (int i = 0; i < chr2->nbgene(); i++)
    {
        chr2->sigma_update(i, (0.5*(chrmat1.get_sigma(i) + chrmat2.get_sigma(i))));
    }
}

template <typename T>
void RealValuedWholeArithmeticRecombination(const galgo::Population<T>& x, galgo::CHR<T>& chr1, galgo::CHR<T>& chr2)
{
    // choosing randomly 2 chromosomes from mating population
    int idx1 = galgo::uniform<int>(0, x.matsize());
    int idx2 = galgo::uniform<int>(0, x.matsize());

    double r = chr1->recombination_ratio();
    const galgo::Chromosome<T>& chrmat1 = *x[idx1];
    const galgo::Chromosome<T>& chrmat2 = *x[idx2];

    for (int i = 0; i < chr1->nbgene(); i++)
    {
        chr1->initGene(i, (T)(r * chrmat2.get_value(i) + (1.0 - r) * chrmat1.get_value(i)));
    }
    for (int i = 0; i < chr2->nbgene(); i++)
    {
        chr2->initGene(i, (T)(r * chrmat1.get_value(i) + (1.0 - r) * chrmat2.get_value(i)));
    }

    // Transmit sigma
    for (int i = 0; i < chr1->nbgene(); i++)
    {
        chr1->sigma_update(i, 0.5*(chrmat1.get_sigma(i) + chrmat2.get_sigma(i)));
    }
    for (int i = 0; i < chr2->nbgene(); i++)
    {
        chr2->sigma_update(i, 0.5*(chrmat1.get_sigma(i) + chrmat2.get_sigma(i)));
    }
}


// one-point random cross-over of 2 chromosomes
template <typename T>
void P1XO(const galgo::Population<T>& x, galgo::CHR<T>& chr1, galgo::CHR<T>& chr2)
{
   // choosing randomly 2 chromosomes from mating population
   int idx1 = galgo::uniform<int>(0, x.matsize());
   int idx2 = galgo::uniform<int>(0, x.matsize());
   // choosing randomly a position for cross-over
   int pos = galgo::uniform<int>(0, chr1->size());
   // transmitting portion of bits to new chromosomes
   chr1->setPortion(*x[idx1], 0, pos);
   chr2->setPortion(*x[idx2], 0, pos);
   chr1->setPortion(*x[idx2], pos + 1);
   chr2->setPortion(*x[idx1], pos + 1);

   const galgo::Chromosome<T>& chrmat1 = *x[idx1];
   const galgo::Chromosome<T>& chrmat2 = *x[idx2];

   // Transmit sigma
   for (int i = 0; i < chr1->nbgene(); i++)
   {
       chr1->sigma_update(i, 0.5*(chrmat1.get_sigma(i)+chrmat2.get_sigma(i) ));
   }
   for (int i = 0; i < chr2->nbgene(); i++)
   {
       chr2->sigma_update(i, 0.5*(chrmat1.get_sigma(i) + chrmat2.get_sigma(i) ));
   }
}

/*-------------------------------------------------------------------------------------------------*/

// two-point random cross-over of 2 chromosomes
template <typename T, int...N>
void P2XO(const galgo::Population<T>& x, galgo::CHR<T>& chr1, galgo::CHR<T>& chr2)
{
   // choosing randomly 2 chromosomes from mating population
   int idx1 = galgo::uniform<int>(0, x.matsize());
   int idx2 = galgo::uniform<int>(0, x.matsize());
   // choosing randomly 2 positions for cross-over
   int pos1 = galgo::uniform<int>(0, chr1->size());
   int pos2 = galgo::uniform<int>(0, chr1->size());
   // ordering these 2 random positions
   int m = std::min(pos1,pos2);
   int M = std::max(pos1,pos2);
   // transmitting portion of bits new chromosomes
   chr1->setPortion(*x[idx1], 0, m);   
   chr2->setPortion(*x[idx2], 0, m);
   chr1->setPortion(*x[idx2], m + 1, M);
   chr2->setPortion(*x[idx1], m + 1, M);
   chr1->setPortion(*x[idx1], M + 1);
   chr2->setPortion(*x[idx2], M + 1);

   // Transmit sigma
   // *x[idx1] is operator[](int pos) is access element in mating population at position pos
   const galgo::Chromosome<T>& chrmat1 = *x[idx1];
   const galgo::Chromosome<T>& chrmat2 = *x[idx2];
   for (int i = 0; i < chr1->nbgene(); i++)
   {
       chr1->sigma_update(i, 0.5*(chrmat1.get_sigma(i) + chrmat2.get_sigma(i)));
   }
   for (int i = 0; i < chr2->nbgene(); i++)
   {
       chr2->sigma_update(i, 0.5*(chrmat1.get_sigma(i) + chrmat2.get_sigma(i)));
   }
}

/*-------------------------------------------------------------------------------------------------*/

// uniform random cross-over of 2 chromosomes
template <typename T>
void UXO(const galgo::Population<T>& x, galgo::CHR<T>& chr1, galgo::CHR<T>& chr2)
{
   // choosing randomly 2 chromosomes from mating population
   int idx1 = galgo::uniform<int>(0, x.matsize());
   int idx2 = galgo::uniform<int>(0, x.matsize());

   for (int j = 0; j < chr1->size(); ++j) {
      // choosing 1 of the 2 chromosomes randomly
      if (galgo::proba(galgo::rng) < 0.50) {
         // adding its jth bit to new chromosome
         chr1->addBit(x[idx1]->getBit(j));
         chr2->addBit(x[idx2]->getBit(j));
      } else {
         // adding its jth bit to new chromosomes
         chr1->addBit(x[idx2]->getBit(j));
         chr2->addBit(x[idx1]->getBit(j));
      }
   }

   // Transmit sigma
   // *x[idx1] is operator[](int pos) is access element in mating population at position pos
   const galgo::Chromosome<T>& chrmat1 = *x[idx1];
   const galgo::Chromosome<T>& chrmat2 = *x[idx2];
   for (int i = 0; i < chr1->nbgene(); i++)
   {
       chr1->sigma_update(i, 0.5*(chrmat1.get_sigma(i) + chrmat2.get_sigma(i)));
   }
   for (int i = 0; i < chr2->nbgene(); i++)
   {
       chr2->sigma_update(i, 0.5*(chrmat1.get_sigma(i) + chrmat2.get_sigma(i)));
   }
}

/*-------------------------------------------------------------------------------------------------*/
template <typename T>
void FixedParameter(galgo::Population<T>& x)
{
    std::vector<galgo::CHR<T>>& np = x.get_newpop();
    for (size_t i = 0; i < x.matsize(); i++)
    {
        for (int j = 0; j < x.ga_algo()->force_value_flag.size(); j++)
        {
            if (x.ga_algo()->force_value_flag[j])
            {
                np[i]->initGene(j, x.ga_algo()->force_value[j]);
                if (np[i]->get_value(j) != x.ga_algo()->force_value[j])
                {
                    int debug = 1;
                    debug++;
                    std::cout << "ERROR - Invalid decode/encode desired_value:" << x.ga_algo()->force_value[j] << " set_value: " << np[i]->get_value(j) << "\n";
                }
            }
        }
    }
}

// MUTATION METHODS

/*-------------------------------------------------------------------------------------------------*/


// boundary mutation: replacing a chromosome gene by its lower or upper bound
template <typename T>
void BDM(galgo::CHR<T>& chr)
{ 
   double mutrate = chr->mutrate();
   if (mutrate == 0.0) return;

   // getting chromosome lower bound(s)
   const std::vector<T>& lowerBound = chr->lowerBound();
   // getting chromosome upper bound(s)
   const std::vector<T>& upperBound = chr->upperBound();

   // looping on number of genes
   for (int i = 0; i < chr->nbgene(); ++i) {
      // generating a random probability
      if (galgo::proba(galgo::rng) <= mutrate) {
         // generating a random probability
         if (galgo::proba(galgo::rng) < .5) {
            // replacing ith gene by lower bound
            chr->initGene(i, lowerBound[i]);
         } else {  
            // replacing ith gene by upper bound
            chr->initGene(i, upperBound[i]);
         }
      }     
   }
}

/*-------------------------------------------------------------------------------------------------*/
// single point mutation: flipping a chromosome bit
template <typename T>
void SPM(galgo::CHR<T>& chr)
{ 
   double mutrate = chr->mutrate();
   if (mutrate == 0.0) return;

   // looping on chromosome bits
   for (int i = 0; i < chr->size(); ++i) 
   {
      // generating a random probability
      if (galgo::proba(galgo::rng) <= mutrate)
      {
         // flipping ith bit
         chr->flipBit(i);  
      }     
   }
}


template <typename T>
void GAM_UncorrelatedOneStepSizeFixed(galgo::CHR<T>& chr)
{
    double mutrate = chr->mutrate();
    if (mutrate == 0.0) return;

    const std::vector<T>& lowerBound = chr->lowerBound();
    const std::vector<T>& upperBound = chr->upperBound();

    double n = chr->nbgene();
    double tau = 1.0 / pow(n, 0.50);
    
    std::normal_distribution < double > distribution01(0.0, 1.0);

    // looping on number of genes
    for (int i = 0; i < chr->nbgene(); ++i)
    {
        // generating a random probability
        if (galgo::proba(galgo::rng) <= mutrate)
        {
            T value = chr->get_value(i);
            double sigma = chr->get_sigma(i);

            if (sigma < 0.00000000001) // first time
            {
                sigma = chr->mutinfo()._sigma;
                if (sigma < chr->mutinfo()._sigma_lowest)
                    sigma = chr->mutinfo()._sigma_lowest;
                chr->sigma_update(i, sigma);
            }

            double newsigma = sigma * std::exp(tau * distribution01(galgo::rng));
            if (newsigma < chr->mutinfo()._sigma_lowest)
                newsigma = chr->mutinfo()._sigma_lowest;
            chr->sigma_update(i, newsigma);

            double norm01 = distribution01(galgo::rng);
            T step = (T)(newsigma * norm01);

            T newvalue = (T)(std::min<T>(std::max<T>(value + step, lowerBound[i]), upperBound[i]));
            chr->initGene(i, newvalue);
        }
    }
}

template <typename T>
void GAM_UncorrelatedOneStepSizeBoundary(galgo::CHR<T>& chr)
{
    double mutrate = chr->mutrate();
    if (mutrate == 0.0) return;

    const std::vector<T>& lowerBound = chr->lowerBound();
    const std::vector<T>& upperBound = chr->upperBound();

    double n = (double)chr->nbgene();
    double tau = 1.0 / pow(n, 0.50);

    std::normal_distribution<double> distribution01(0.0, 1.0);

    // looping on number of genes
    for (int i = 0; i < chr->nbgene(); ++i)
    {
        // generating a random probability
        if (galgo::proba(galgo::rng) <= mutrate)
        {
            T value = chr->get_value(i);
            double sigma = chr->get_sigma(i);

            if (sigma < 0.00000000001) // first time
            {
                sigma = (upperBound[i] - lowerBound[i]) * chr->mutinfo()._ratio_boundary;
                if (sigma < chr->mutinfo()._sigma_lowest)
                    sigma = chr->mutinfo()._sigma_lowest;
                chr->sigma_update(i, sigma);
            }

            double newsigma = sigma * std::exp(tau * distribution01(galgo::rng));
            if (newsigma < chr->mutinfo()._sigma_lowest)
                newsigma = chr->mutinfo()._sigma_lowest;
            chr->sigma_update(i, newsigma);

            double norm01 = distribution01(galgo::rng);
            T step = (T)(newsigma * norm01);

            T newvalue = (T)(std::min<T>(std::max<T>(value + step, lowerBound[i]), upperBound[i]));
            chr->initGene(i, newvalue);
        }
    }
}

template <typename T>
void GAM_UncorrelatedNStepSize(galgo::CHR<T>& chr)
{
    double mutrate = chr->mutrate();
    if (mutrate == 0.0) return;

    const std::vector<T>& lowerBound = chr->lowerBound();
    const std::vector<T>& upperBound = chr->upperBound();

    std::normal_distribution<double> distribution01(0.0, 1.0);

    double n = (double)chr->nbgene();
    double tau1 = 1.0 / pow(2.0*n, 0.50);
    double tau2 = 1.0 / pow(2.0*pow(n,0.50), 0.50);

    // looping on number of genes
    for (int i = 0; i < chr->nbgene(); ++i)
    {
        // generating a random probability
        if (galgo::proba(galgo::rng) <= mutrate)
        {
            T value = chr->get_value(i);
            double sigma = chr->get_sigma(i);

            if (sigma < 0.00000000001) // never copied from parent
            {
                sigma = chr->mutinfo()._sigma;
                if (sigma < chr->mutinfo()._sigma_lowest)
                    sigma = chr->mutinfo()._sigma_lowest;
                chr->sigma_update(i, sigma);
            }
            else
            {
                double factor1 = std::exp(tau1 * distribution01(galgo::rng));
                double factor2 = std::exp(tau2 * distribution01(galgo::rng));

                double newsigma = sigma * factor1 * factor2;
                if (newsigma < chr->mutinfo()._sigma_lowest)
                    newsigma = chr->mutinfo()._sigma_lowest;

                chr->sigma_update(i, newsigma);
                sigma = newsigma;
            }

            double norm01 = distribution01(galgo::rng);
            T newvalue = (T)(std::min<T>(std::max<T>(value + (T)(sigma * norm01), lowerBound[i]), upperBound[i]));
            chr->initGene(i, newvalue);
        }
    }
}

template <typename T>
void GAM_UncorrelatedNStepSizeBoundary(galgo::CHR<T>& chr)
{
    double mutrate = chr->mutrate();
    if (mutrate == 0.0) return;

    const std::vector<T>& lowerBound = chr->lowerBound();
    const std::vector<T>& upperBound = chr->upperBound();

    std::normal_distribution<double> distribution01(0.0, 1.0);

    double n = (double)chr->nbgene();
    double tau1 = 1.0 / pow(2.0*n, 0.50);
    double tau2 = 1.0 / pow(2.0*pow(n, 0.50), 0.50);

    // looping on number of genes
    for (int i = 0; i < chr->nbgene(); ++i)
    {
        // generating a random probability
        if (galgo::proba(galgo::rng) <= mutrate)
        {
            T value = chr->get_value(i);
            double sigma = chr->get_sigma(i);

            if (sigma < 0.00000000001) // never copied from parent
            {
                sigma = (upperBound[i] - lowerBound[i]) * chr->mutinfo()._ratio_boundary;
                if (sigma < chr->mutinfo()._sigma_lowest)
                    sigma = chr->mutinfo()._sigma_lowest;
                chr->sigma_update(i, sigma);
            }
            else
            {
                double factor1 = std::exp(tau1 * distribution01(galgo::rng));
                double factor2 = std::exp(tau2 * distribution01(galgo::rng));

                double newsigma = sigma * factor1 * factor2;
                if (newsigma < chr->mutinfo()._sigma_lowest)
                    newsigma = chr->mutinfo()._sigma_lowest;

                chr->sigma_update(i, newsigma);
                sigma = newsigma;
            }

            double norm01 = distribution01(galgo::rng);
            T newvalue = (T)(std::min<T>(std::max<T>(value + (T)(sigma * norm01), lowerBound[i]), upperBound[i]));
            chr->initGene(i, newvalue);
        }
    }
}

/*-------------------------------------------------------------------------------------------------*/
// Gaussian mutation: replacing a chromosome gene by near guassian value
template <typename T>
void GAM_sigma_adapting_per_generation(galgo::CHR<T>& chr)
{
    double mutrate = chr->mutrate();
    if (mutrate == 0.0) return;

    const std::vector<T>& lowerBound = chr->lowerBound();
    const std::vector<T>& upperBound = chr->upperBound();

    std::normal_distribution<double> distribution01(0.0, 1.0);

    // looping on number of genes
    for (int i = 0; i < chr->nbgene(); ++i)
    {
        // generating a random probability
        if (galgo::proba(galgo::rng) <= mutrate)
        {
            T value = chr->get_value(i);
            double sigma = ((double)(upperBound[i] - lowerBound[i])) * chr->mutinfo()._ratio_boundary;
            if (sigma < chr->mutinfo()._sigma_lowest)
                sigma = chr->mutinfo()._sigma_lowest;

            double norm01;

            // sigma decreasing blindly with number generation produced
            for (int z = 1; z < chr->nogen() / 2; z++)
            {
                norm01 = distribution01(galgo::rng);
                sigma = std::max(double(0), sigma * exp(norm01));
            }

            std::normal_distribution<double> distribution((double)value, sigma);
            double norm = distribution(galgo::rng);
            T newvalue = (T)(std::min(std::max((T)norm, lowerBound[i]), upperBound[i]));
            chr->initGene(i, newvalue);
        }
    }
}
/*-------------------------------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------------------------------*/
// Gaussian mutation: replacing a chromosome gene by near gaussian value
template <typename T>
void GAM_sigma_adapting_per_mutation(galgo::CHR<T>& chr)
{
    double mutrate = chr->mutrate();
    if (mutrate == 0.0) return;

    const std::vector<T>& lowerBound = chr->lowerBound();
    const std::vector<T>& upperBound = chr->upperBound();

    std::normal_distribution<double> distribution01(0.0, 1.0);

    // looping on number of genes
    for (int i = 0; i < chr->nbgene(); ++i)
    {
        // generating a random probability
        if (galgo::proba(galgo::rng) <= mutrate)
        {
            T value = chr->get_value(i);
            double sigma = chr->get_sigma(i);

            if (sigma < 0.00000000001) // never copied from parent
            {
                sigma = ((double)(upperBound[i] - lowerBound[i])) * chr->mutinfo()._ratio_boundary;
                if (sigma < chr->mutinfo()._sigma_lowest)
                    sigma = chr->mutinfo()._sigma_lowest;
                chr->sigma_update(i, sigma);
            }

            std::normal_distribution<double> distribution((double)value, sigma);
            double norm = distribution(galgo::rng);
            T new_value = (T)(std::min(std::max((T)norm, lowerBound[i]), upperBound[i]));
            chr->initGene(i, new_value);
        }
    }
}
/*-------------------------------------------------------------------------------------------------*/


// uniform mutation: replacing a chromosome gene by a new one
template <typename T>
void UNM(galgo::CHR<T>& chr)
{ 
   double mutrate = chr->mutrate();
   if (mutrate == 0.0) return;

   // looping on number of genes
   for (int i = 0; i < chr->nbgene(); ++i) {
      // generating a random probability
      if (galgo::proba(galgo::rng) <= mutrate) {
         // replacing ith gene by a new one
         chr->setGene(i);  
      }     
   }
}

/*-------------------------------------------------------------------------------------------------*/

// ADAPTATION TO CONSTRAINT(S) METHODS

/*-------------------------------------------------------------------------------------------------*/

// adapt population to genetic algorithm constraint(s)
template <typename T>
void DAC(galgo::Population<T>& x)
{
   // getting worst population objective function total result
   double worstTotal = x.getWorstTotal();

   for (auto it = x.begin(), end = x.end(); it != end; ++it) 
   {
      // computing element constraint value(s) 
      const std::vector<double>& cst = (*it)->getConstraint();

      // adapting fitness if any constraint violated
      if (std::any_of(cst.cbegin(), cst.cend(), [](double x)->bool{return x >= 0.0;}))
      {
         (*it)->fitness = worstTotal - std::accumulate(cst.cbegin(), cst.cend(), 0.0);
      }
   }
} 

//================================================================================================= 

#endif
