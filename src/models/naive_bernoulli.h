
class naive_bernoulli
{
    public:

    class network
    {
        public:
        mat adj;

        network(Rcpp::List & network_from_R)
        {
            mat adj_orig = network_from_R["adjacency"];
            adj = adj_orig;
        }
    };

    struct is_sbm_symmetric
    {
        enum { value=false };
    };

    /* parameters */
    unsigned int n_parameters;
    mat pi;

    naive_bernoulli(SBM & membership)
    {
        n_parameters = membership.Z.n_cols * membership.Z.n_cols;
        pi.set_size(membership.Z.n_cols,membership.Z.n_cols);
        pi.fill(.5);
    }
    
    naive_bernoulli(LBM & membership)
    {
        n_parameters = membership.Z1.n_cols * membership.Z2.n_cols;
        pi.set_size(membership.Z1.n_cols,membership.Z2.n_cols);
        pi.fill(.5);
    }

    inline
    Rcpp::List prediction(SBM & membership)
    {
        mat pred(membership.Z.n_rows, membership.Z.n_rows);
        pred.fill(0);

        for(unsigned int i=0;i<pred.n_rows;i++)
            for(unsigned int j=0;i<pred.n_cols;j++)
                for(unsigned int q=0;q<membership.Z.n_cols;q++)
                    for(unsigned int l=0;l<membership.Z.n_cols;l++)
                        pred(i,j) +=   membership.Z(i,q)
                                     * membership.Z(j,l)
                                     * pi(q,l);
        Rcpp::List Lpred;
        Lpred["adjacency"] = pred;

        return Lpred;
    }
    
    inline
    Rcpp::List prediction(LBM & membership)
    {
        mat pred(membership.Z1.n_rows, membership.Z2.n_rows);
        pred.fill(0);

        for(unsigned int i=0;i<pred.n_rows;i++)
            for(unsigned int j=0;i<pred.n_cols;j++)
                for(unsigned int q=0;q<membership.Z1.n_cols;q++)
                    for(unsigned int l=0;l<membership.Z2.n_cols;l++)
                        pred(i,j) +=   membership.Z1(i,q)
                                     * membership.Z2(j,l)
                                     * pi(q,l);
        Rcpp::List Lpred;
        Lpred["adjacency"] = pred;
        return Lpred;
    }

    inline
    Rcpp::List export_to_R()
    {
        Rcpp::List values;
        values["pi"] = pi;
        values["n_parameters"] = n_parameters;

        return values;
    }

    inline
    vec to_vector()
    {
        return reshape(pi,n_parameters,1);
    }

    naive_bernoulli(SBM & membership, const vec & vectorized)
    {
        n_parameters = membership.Z.n_cols * membership.Z.n_cols;
        pi = reshape(vectorized, membership.Z.n_cols, membership.Z.n_cols);
    }
    
    naive_bernoulli(LBM & membership, const vec & vectorized)
    {
        n_parameters = membership.Z1.n_cols * membership.Z2.n_cols;
        pi = reshape(vectorized, membership.Z1.n_cols, membership.Z2.n_cols);
    }
};

inline
double logf(naive_bernoulli & model, naive_bernoulli::network & net, unsigned int i, unsigned int j, unsigned int q, unsigned int l)
{
    return net.adj(i,j)*log(model.pi(q,l)) + (1-net.adj(i,j))*log(1-model.pi(q,l));
}

inline
double grad_logf(naive_bernoulli & model, naive_bernoulli::network & net, unsigned int i, unsigned int j, unsigned int q, unsigned int l, unsigned int param)
{
    if( param % model.pi.n_rows == q )
    {
        if( param / model.pi.n_rows == l)
            return net.adj(i,j)/(model.pi(q,l)) - (1-net.adj(i,j))/(1-model.pi(q,l));
    }
    
    return 0;
}

inline
double maximum_step_in_direction(naive_bernoulli & model, vec direction)
{
    vec v = model.to_vector();

    double amax=1;

    for(unsigned int k=0; k<direction.n_elem; k++)
    {
        double b = direction(k)>0 ?
                                    (1-v(k))/direction(k)
                                  :
                                    -v(k)/direction(k);
        if(b<amax)
            amax=b;
    }

    return amax;            
    
}