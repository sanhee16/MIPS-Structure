int hit_inst, miss_inst, hit_data, miss_data, hit_l2, miss_l2;


void calc(int lv2_cache){
/*
   assume
	if hit = 1 cycle, 0.2 ns(L1 cache)
	if hit = 18 cycle, 3.6 ns(L2 cache)
	if miss = 180 cycle, 50 ns(main memory)

  */
	float result_hit, result_miss;
	float time_inst, time_data, time_lv2;
   	int	cyc_inst, cyc_data, cyc_lv2;
	time_inst=0.0; time_data=0.0; time_lv2=0.0;
	cyc_inst=0; cyc_data=0; cyc_lv2=0;



	result_hit=((float)hit_inst/(float)(hit_inst+miss_inst))* 100;
	result_miss=((float)miss_inst/(float)(hit_inst+miss_inst))*100;

	cyc_inst = hit_inst*4+miss_inst*162;

	if(lv2_cache){
		cyc_inst=hit_inst*4+miss_inst*14;	
	}
	time_inst =cyc_inst*0.2;

	printf("\ninst hit  %d  \t\t inst miss  %d\n",hit_inst, miss_inst);
	printf("inst hit rate %f %% \t inst miss rate %f %% \n",result_hit,result_miss);
	printf("inst time : %f ns\t cycle : %d cyc\n",time_inst,cyc_inst);

	result_hit=((float)hit_data/(float)(hit_data+miss_data))*100;
	result_miss=((float)miss_data/(float)(hit_data+miss_data))*100;

	cyc_data=hit_data*4+miss_data*162;

	if(lv2_cache){
		cyc_data=hit_data*4+miss_data*14;
	}
	time_data = cyc_data*0.2;

	printf("\ndata hit   %d  \t\t data miss  %d\n",hit_data, miss_data);
	printf("data hit rate %f %% \t data miss rate %f %% \n",result_hit,result_miss);
	printf("data time : %f ns\t cycle : %d cyc\n",time_data,cyc_data);

	if(lv2_cache){
		result_hit=((float)hit_l2/(float)(hit_l2+miss_l2))*100;
		result_miss=((float)miss_l2/(float)(hit_l2+miss_l2))*100;

		cyc_lv2 = hit_l2*14+miss_l2*162;
		time_lv2 = cyc_lv2*0.2;
		
		printf("\nLv2 cache hit  %d  \t\t Lv2 cache miss  %d\n",hit_l2, miss_l2);
		printf("Lv2 cache hit rate %f %% \t L2 cache miss rate %f %% \n",result_hit,result_miss);
		printf("Lv2 cache time : %f ns\t cycle : %d cyc\n",time_lv2,cyc_lv2);
	}
	float total_time = time_inst + time_data + time_lv2;	
	int total_cyc = cyc_inst + cyc_data + cyc_lv2;
	printf("\ntotal time %f ns\t total cycle %d cyc\n\n",total_time, total_cyc);
	
	return ;
}
