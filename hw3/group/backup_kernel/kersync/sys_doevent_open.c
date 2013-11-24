asmlinkage long sys_doevent_wait(long event_id)
{
	/*lock global lock*/

	event_t * event = find_event_by_id(event_id);		/*find event by event id*/
	if(event==NULL)
	{
		/*unlock global lock*/
		return -1;
	}
	event->count++;										/*increase the num of process using the event*/

	/*unlock global lock*/

	/*lock event lock*/

	if(event->invalid)									/*if already close*/
	{
		if(event->count==1)								/*if last one of use*/
			kfree(event);
		count--;										/*after use of the event*/
		return -1;
	}
	sleep_on(&event->Q);								/*sleep on event*/
	event->count--;										/*after use of the event*/

	/*unlock event lock*/
	
	return 0;
}

asmlinkage long sys_doevent_sig(long event_id)
{
	/*lock global lock*/

	event_t * event = find_event_by_id(event_id);		/*find event by event id*/
	if(event==NULL)
	{
		/*unlock global event*/
		return -1;
	}
	event->count++;										/*increase the num of process using the event*/

	/*unlock global lock*/

	/*lock event lock*/

	if(event->invalid)									/*if already close*/
	{
		if(event->count==1)								/*if last one of use*/
			kfree(event);
		count--;										/*after use of the event*/
		return -1;
	}
	wake_up(&event->Q);									/*signal the queue*/
	event->count--;										/*after use of the event*/

	/*unlock event lock*/
	
	return 0;
}

asmlinkage long sys_doevent_close(long event_id)
{
	/*lock global lock*/

	event_t * event = find_event_by_id(event_id);		/*find event by event id*/
	if(event == NULL)
	{
		/*unlock global lock*/
		return -1;
	}
	list_del(&event->list);

	/*unlock global lock*/
	
	/*lock event lock*/
		
		wake_up(&event->Q);								/*signal the queue*/
		event->invalid = 1;
		if(event->count==0)
			kfree(event);
		
	/*unlock event lock*/
	
	return 0;
}


